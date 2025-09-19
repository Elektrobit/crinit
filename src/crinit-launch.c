// SPDX-License-Identifier: MIT
/**
 * @file crinit-launch.c
 * @brief Launcher helper needed to start programs with different user / group permissions.
 *
 * Program usage info:
 *
 * ~~~
 * USAGE: crinit-launch --cmd=/path/to/targetcmd [--user=UID --groups=GID[,SGID1,SGID2]] -- [TARGET_COMMAND_ARGUMENTS]
 * where ACTION must be exactly one of (including specific options/parameters):
 *    cmd Path to the program to launch.
 *   user UID of the user to be used to start the specified command. If not given, the user of the crinit process is
 * used. groups Comma separated list of GIDs that shall be used to start the specified command. The first one will be
 * used as the primary group, all others as suplimentary groups. If not given the group of the crinit process is used.
 *
 * After the delimiter -- the arguments of the specifed command can be given, if there are any.
 * General Options:
 *       --help/-h    - Print this help.
 *       --version/-V - Print version information about crinit-launch.
 * ~~~
 */

#include <getopt.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef ENABLE_CAPABILITIES
#include "capabilities.h"
#endif
#ifdef ENABLE_CGROUP
#include "cgroup.h"
#endif
#include "crinit-version.h"
#include "logio.h"

static void crinitPrintVersion(void) {
    fprintf(stderr, "Crinit version %s\n", crinitGetVersionString());
}

static void crinitPrintUsage(void) {
    fprintf(
        stderr,
        "USAGE: crinit-launch --cmd=/path/to/targetcmd [--user=UID --groups=GID[,SGID1,SGID2]] "
#ifdef ENABLE_CGROUP
        "--cgroup=<cgroup> "
#endif
#ifdef ENABLE_CAPABILITIES
        "--caps=bitmask -- "
#endif
        "[TARGET_COMMAND_ARGUMENTS]\n"
        "  where ACTION must be exactly one of (including specific options/parameters):\n"
        "    cmd Path to the program to launch.\n"
        "    user UID of the user to be used to start the specified command. If not given, the user of the crinit "
        "process is used.\n"
        "    groups Comma separated list of GIDs that shall be used to start the specified command. The first one "
        "will\n"
        "       be used as the primary group, all others as suplimentary groups. If not given the group of the crinit "
        "process is used.\n"
#ifdef ENABLE_CGROUP
        "    cgroup Name of the cgroup that shall be used to start the target process in. If the cgroup has a parent "
        "cgroup, \n"
        "that cgroup has to be given as well. Example: \"crinit.cg/my.cg\"\n"
#endif
#ifdef ENABLE_CAPABILITIES
        "    caps Bitmask in hexadecimal format that represents all capabilities that shall be added.\n"
        "       Bit positions correspond to capability values that are defined by the kernel in <linux/capability.h>.\n"
        "       E.g. setting capability CAP_SETGID which has a value 6) would require a bitmap with value 0x40.\n"
#endif
        "\n"
        " After the delimiter -- the arguments of the specifed command can be given, if there are any.\n"
        " General Options:\n"
        "      --help/-h    - Print this help.\n"
        "      --version/-V - Print version information about crinit-launch.\n");
}

int crinitExtractGroups(char *input, gid_t **groups, size_t *groupSize) {
    char *in = input;
    char *rest = NULL;
    char *token = NULL;

    *groupSize = 0;
    while ((token = strchr(in, ','))) {
        if (token) {
            (*groupSize)++;
            in = token + 1;
        }
    }
    (*groupSize)++;

    *groups = calloc(*groupSize, sizeof(gid_t));
    if (!*groups) {
        crinitErrPrint("Failed to allocate memory for group input parsing.\n");
        return -1;
    }

    in = input;
    size_t i = 0;
    while ((token = strtok_r(in, ",", &rest))) {
        in = NULL;
        if (*token != '\0') {
            char *endptr = NULL;
            errno = 0;
            (*groups)[i] = strtoul(token, &endptr, 10);
            if (endptr != token && *endptr == '\0' && errno == 0) {
                i++;
            } else {
                crinitErrPrint("Malformed input: %s.\n", input);
                return -1;
            }
        } else {
            crinitErrPrint("Malformed input: %s.\n", input);
            return -1;
        }
    }

    if (i != *groupSize) {
        crinitErrPrint("Parsing count mismatch. %lu != %lu\n", *groupSize, i);
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    const struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                         {"version", no_argument, 0, 'V'},
                                         {"cmd", required_argument, 0, 'c'},
                                         {"user", required_argument, 0, 'u'},
                                         {"groups", required_argument, 0, 'g'},
#ifdef ENABLE_CAPABILITIES
                                         {"caps", required_argument, 0, 'p'},
#endif
#ifdef ENABLE_CGROUP
                                         {"cgroup", optional_argument, 0, 'r'},
#endif
                                         {0, 0, 0, 0}};

#ifdef ENABLE_CAPABILITIES
    unsigned long caps = 0;
#endif
#ifdef ENABLE_CGROUP
    char *targetCgroup = NULL;
#endif
    gid_t *groups = NULL;
    size_t groupSize = 0;
    uid_t user = -1;
    char *cmd = NULL;
    bool userFound = false;
    char *argvNewBuffer = NULL;
    char **argvNew = NULL;

#ifdef ENABLE_CAPABILITIES
#define CAPOPTS ":p"
#else
#define CAPOPTS ""
#endif
#ifdef ENABLE_CGROUP
#define CGROUPOPTS ":r"
#else
#define CGROUPOPTS ""
#endif
    const char *opts = "hc:u:g:V" CAPOPTS CGROUPOPTS;

    while (true) {
        opt = getopt_long(argc, argv, opts, longOptions, NULL);
        if (opt == -1) {
            break;
        }
        switch (opt) {
            case 'c':
                if (cmd) {
                    crinitErrPrint("Parameter --cmd may only be given once.\n");
                    crinitPrintUsage();
                    goto failureExit;
                }
                cmd = strdup(optarg);
                break;
            case 'u': {
                char *endptr = NULL;
                errno = 0;
                user = strtoul(optarg, &endptr, 10);
                if (endptr == NULL || endptr == optarg || *endptr != '\0' || errno != 0) {
                    crinitErrPrint("Malformed input for user parameter: %s.\n", optarg);
                    goto failureExit;
                }
                userFound = true;
            } break;
            case 'g':
                if (groups) {
                    crinitErrPrint("Parameter --groups may only be given once.\n");
                    crinitPrintUsage();
                    goto failureExit;
                }
                if (crinitExtractGroups(optarg, &groups, &groupSize) != 0) {
                    crinitErrPrint("Failed to extract groups. Input: %s\n", optarg);
                    goto failureExit;
                }
                break;
#ifdef ENABLE_CAPABILITIES
            case 'p':
                caps = strtoul(optarg, NULL, 16);
                crinitInfoPrint("Provided capability set mask: %#lx (option --caps: %s)", caps, optarg);
                break;
#endif
#ifdef ENABLE_CGROUP
            case 'r':
                if (targetCgroup) {
                    crinitErrPrint("Parameter --cgroup may only be given once.\n");
                    crinitPrintUsage();
                    goto failureExit;
                }
                targetCgroup = strdup(optarg);
                break;
#endif
            case 'V':
                crinitPrintVersion();
#ifdef ENABLE_CGROUP
                free(targetCgroup);
#endif
                free(groups);
                free(cmd);
                return EXIT_SUCCESS;
            case 'h':
            case '?':
            default:
                crinitPrintUsage();
                goto failureExit;
        }
    }

    if (cmd == NULL) {
        crinitErrPrint("Option --cmd not provided.");
        crinitPrintUsage();
        goto failureExit;
    }

    size_t argvNewSize = strlen(cmd) + 1;
    for (int i = optind; i < argc; i++) {
        argvNewSize += strlen(argv[i]) + 1;
    }

    argvNewBuffer = calloc(argvNewSize + 1, sizeof(char));
    if (!argvNewBuffer) {
        crinitErrPrint("Failed to alloc memory for target argv buffer.\n");
        goto failureExit;
    }
    argvNew = calloc(argc - optind + 2, sizeof(char *));
    if (!argvNew) {
        crinitErrPrint("Failed to alloc memory for target argv.\n");
        goto failureExit;
    }
    char *p = argvNewBuffer;
    strcpy(p, cmd);
    argvNew[0] = p;
    p += strlen(cmd) + 1;
    int idx = 1;
    for (int i = optind; i < argc; i++) {
        strcpy(p, argv[i]);
        argvNew[idx] = p;
        p += strlen(argv[i]) + 1;
        idx++;
    }

#ifdef ENABLE_CAPABILITIES
    crinitInfoPrint("Retain permitted capabilities to prevent its clearance by switching to non privileged UID.");
    if (crinitCapRetainPermitted() == -1) {
        goto failureExit;
        return -1;
    }
#endif

    if (groupSize) {
        if (setgroups(0, NULL) != 0) {  // Drop all current supplementary groups
            crinitErrPrint("Failed to drop all initial supplementary groups.\n");
            goto failureExit;
        }
        if (setgid(groups[0]) != 0) {
            crinitErrPrint("Failed to set group to ID %du.\n", groups[0]);
            goto failureExit;
        }

        if (groupSize > 1) {
            if (setgroups(groupSize - 1, groups + 1) != 0) {
                crinitErrPrint("Failed to set supplementary groups.\n");
                goto failureExit;
            }
        }
    }

    if (userFound) {
        if (setuid(user) != 0) {
            crinitErrPrint("Failed to set UID to target %d.\n", user);
            goto failureExit;
        }
    }

#ifdef ENABLE_CAPABILITIES
    crinitInfoPrint("Prepare setting ambient capabilities: Set inheritable capabilities accordingly.");
    if (crinitCapSetInheritable(caps) == -1) {
        goto failureExit;
    }

    if (crinitCapSetAmbient(caps) == -1) {
        goto failureExit;
    }
#endif

#ifdef ENABLE_CGROUP
    if (targetCgroup) {
        const pid_t pid = getpid();
        crinitCgroup_t cgroup;
        cgroup.config = NULL;
        cgroup.groupFd = -1;
        cgroup.parent = NULL;
        cgroup.name = targetCgroup;
        if (crinitCGroupAssignPID(&cgroup, pid) != 0) {
            crinitErrPrint("Failed to switch to target cgroup '%s'.", cgroup.name);
            goto failureExit;
        }
        free(cgroup.name);
    }
#endif

    execvp(cmd, argvNew);

    crinitErrnoPrint("Failed to execvp().\n");

failureExit:
#ifdef ENABLE_CGROUP
    free(targetCgroup);
#endif
    free(groups);
    free(cmd);
    free(argvNewBuffer);
    free(argvNew);
    return EXIT_FAILURE;
}
