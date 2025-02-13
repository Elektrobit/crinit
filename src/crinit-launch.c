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
 *   user UID of the user to be used to start the specified command. If not given, the user of the crinit process is used.
 * groups Comma separated list of GIDs that shall be used to start the specified command. The first one will
 *        be used as the primary group, all others as suplimentary groups. If not given the group of the crinit process is used.
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

#include "logio.h"
#include "version.h"

static void crinitPrintVersion(void) {
    fprintf(stderr, "Crinit version %s\n", crinitGetVersionString());
}

static void crinitPrintUsage(void) {
    fprintf(
        stderr,
        "USAGE: crinit-launch --cmd=/path/to/targetcmd [--user=UID --groups=GID[,SGID1,SGID2]] -- [TARGET_COMMAND_ARGUMENTS]\n"
        "  where ACTION must be exactly one of (including specific options/parameters):\n"
        "    cmd Path to the program to launch.\n"
        "   user UID of the user to be used to start the specified command. If not given, the user of the crinit process is used.\n"
        " groups Comma separated list of GIDs that shall be used to start the specified command. The first one will\n"
        "       be used as the primary group, all others as suplimentary groups. If not given the group of the crinit process is used.\n"
        "\n"
        " After the delimiter -- the arguments of the specifed command can be given, if there are any.\n"
        " General Options:\n"
        "      --help/-h    - Print this help.\n"
        "      --version/-V - Print version information about crinit-launch.\n");
}

int extractGroups(char *input, gid_t **groups, size_t *groupSize) {
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
        if (token) {
            (*groups)[i] = strtoul(token, NULL, 10);
            i++;
        }
    }

    if (i != *groupSize) {
        crinitErrPrint("Parsing count missmatch. %lu != %lu\n", *groupSize, i);
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
                                         {0, 0, 0, 0}};

    gid_t *groups = NULL;
    size_t groupSize = 0;
    uid_t user = -1;
    char *cmd = NULL;
    bool userFound = false;

    while (true) {
        opt = getopt_long(argc, argv, "hc:u:g:V", longOptions, NULL);
        if (opt == -1) {
            break;
        }
        switch (opt) {
            case 'c':
                cmd = strdup(optarg);
                break;
            case 'u':
                user = strtoul(optarg, NULL, 10);
                userFound = true;
                break;
            case 'g':
                if (extractGroups(optarg, &groups, &groupSize) != 0) {
                    crinitErrPrint("Failed to extract groups. Input: %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'V':
                crinitPrintVersion();
                return EXIT_SUCCESS;
            case 'h':
            case '?':
            default:
                crinitPrintUsage();
                return EXIT_FAILURE;
        }
    }

    char *argvNewBuffer = NULL;
    size_t argvNewSize = strlen(cmd) + 1;
    for (int i = optind; i < argc; i++) {
        argvNewSize += strlen(argv[i]) + 1;
    }

    argvNewBuffer = calloc(argvNewSize + 1, sizeof(char));
    if (!argvNewBuffer) {
        crinitErrPrint("Failed to alloc memory for target argv buffer.\n");
        return EXIT_FAILURE;
    }
    char **argvNew = NULL;
    argvNew = calloc(argc - optind + 2, sizeof(char *));
    if (!argvNew) {
        crinitErrPrint("Failed to alloc memory for target argv.\n");
        return EXIT_FAILURE;
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

    if (groupSize) {
        if (setgroups(0, NULL) != 0) {     // Drop all current sublimentary groups
            crinitErrPrint("Failed to drop all initial sublimentary groups.\n");
            return EXIT_FAILURE;
        }
        if (setgid(groups[0]) != 0) {
            crinitErrPrint("Failed to set group to ID %du.\n", groups[0]);
            return EXIT_FAILURE;
        }

        if (groupSize > 1) {
            if (setgroups(groupSize - 1, groups + 1) != 0) {
                crinitErrPrint("Failed to set sublimentary groups.\n");
                return EXIT_FAILURE;
            }
        }
    }

    if (userFound) {
        if (setuid(user) != 0) {
            crinitErrPrint("Failed to set UID to target %d.\n", user);
            return EXIT_FAILURE;
        }
    }

    execvp(cmd, argvNew);

    crinitErrnoPrint("Failed to execvp().\n");

    free(cmd);
    free(argvNewBuffer);
    free(argvNew);
    return EXIT_FAILURE;
}
