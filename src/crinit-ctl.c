// SPDX-License-Identifier: MIT
/**
 * @file crinit-ctl.c
 * @brief Implementation of the crinit-ctl service program using the crinit-client library.
 *
 * Program usage info:
 *
 * ~~~
 * USAGE: crinit-ctl <ACTION> [OPTIONS] <PARAMETER> [PARAMETERS...]
 * where ACTION must be exactly one of (including specific options/parameters):
 *    addtask [-f/--overwrite] [-i/--ignore-deps] [-d/--override-deps "depA:eventA depB:eventB [...]"] <PATH>
 *            - Will add a task defined in the task configuration file at <PATH> (absolute) to Crinit's task database.
 *              '-f/--overwrite' - Lets Crinit know it is fine to overwrite if it has already loaded a task
 *                   with the same name.
 *              '-d/--override-deps <dependency-list>' - Will override the DEPENDS field of the config file
 *                   with what is given as the parameter.
 *              '-i/--ignore-deps' - Shortcut for '--override-deps ""'.
 *  addseries [-f/--overwrite] <PATH>
 *            - Will load a series file from <PATH>. Options set in the new series file take precedence over
 *              current settings.
 *              '-f/--overwrite' - Lets Crinit know it is fine to overwrite if it has already loaded tasks
 *                   with the same name as those in the new series file.
 *     enable <TASK_NAME>
 *            - Removes dependency '@ctl:enable' from the dependency list of <TASK_NAME> if it is present.
 *    disable <TASK_NAME>
 *            - Adds dependency '@ctl:enable' to the dependency list of <TASK_NAME>.
 *       stop <TASK_NAME>
 *            - If the task has a STOP_COMMAND, it will be executed. Otherwise, Crinit sends SIGTERM to the
 *              PID of <TASK_NAME> if the PID is currently known.
 *       kill <TASK_NAME>
 *            - Sends SIGKILL to the PID of <TASK_NAME> if the PID is currently known.
 *    restart <TASK_NAME>
 *            - Resets the status bits of <TASK_NAME> if it is DONE or FAILED.
 *     status <TASK_NAME>
 *            - Queries status bits, PID, and timestamps of <TASK_NAME>. The CTime, STime, and ETime fields
 *              represent the times the task was Created (loaded/parsed), last Started (became running), and
 *              last Ended (failed or is done). If the event has not occurred yet, the timestamp's value will
 *              be 'n/a'.
 *     notify <TASK_NAME> <"SD_NOTIFY_STRING">
 *            - Will send an sd_notify-style status report to Crinit. Only MAINPID and READY are
 *              implemented. See the sd_notify documentation for their meaning.
 *       list
 *            - Print the list of loaded tasks and their status.
 *     reboot
 *            - Will request Crinit to perform a graceful system reboot. crinit-ctl can be symlinked to
 *              reboot as a shortcut which will invoke this command automatically.
 *   poweroff
 *            - Will request Crinit to perform a graceful system shutdown. crinit-ctl can be symlinked to
 *              poweroff as a shortcut which will invoke this command automatically.
 * General Options:
 *       --verbose/-v - Be verbose.
 *       --help/-h    - Print this help.
 *       --version/-V - Print version information about crinit-ctl, the crinit-client library,
 *                      and -- if connection is successful -- the crinit daemon.
 * ~~~
 */
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "crinit-client.h"
#include "logio.h"
#include "version.h"

#define TIME_REPR_MAX_LEN 64                          ///< Maximum length of task time represented as a string.
#define TIME_REPR_PRINTF_FORMAT "%" PRId64 ".%.9lds"  ///< Format string to represent a task timespec.

/**
 * Print usage information.
 *
 * @param prgmPath  The path to the program, usually found in argv[0].
 */
static void crinitPrintUsage(char *prgmPath);
/**
 * Prints a message indicating the versions of crinit-ctl, the client library, and (if connection is successful) the
 * Crinit daemon to stderr.
 */
static void crinitPrintVersion(void);
/**
 * Convert a task state code to a string.
 *
 * @param s  The task state code to convert.
 *
 * @return a string representing the given task status code.
 */
static const char *crinitTaskStateToStr(crinitTaskState_t s);

int main(int argc, char *argv[]) {
    int getoptArgc = argc;
    char **getoptArgv = argv;

    crinitSetPrintPrefix("");

    if (strcmp(basename(argv[0]), "poweroff") != 0 && strcmp(basename(argv[0]), "reboot") != 0) {
        if (argc < 2) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        } else {
            // We need to do this before getopt as we might not have an <ACTION> specified.
            for (int i = 0; i < argc; i++) {
                if (crinitParamCheck(argv[i], "-V", "--version")) {
                    crinitPrintVersion();
                    return EXIT_FAILURE;
                }
            }
        }
        getoptArgc--;
        getoptArgv++;
    }

    int opt;
    const struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                         {"ignore-deps", no_argument, 0, 'i'},
                                         {"override-deps", required_argument, 0, 'd'},
                                         {"overwrite", no_argument, 0, 'f'},
                                         {"verbose", no_argument, 0, 'v'},
                                         {0, 0, 0, 0}};
    bool overwrite = false;
    bool ignoreDeps = false;
    const char *overDeps = NULL;

    bool verbose = false;

    while (true) {
        opt = getopt_long(getoptArgc, getoptArgv, "hd:fiv", longOptions, NULL);
        if (opt == -1) {
            break;
        }
        switch (opt) {
            case 'i':
                ignoreDeps = true;
                break;
            case 'd':
                overDeps = optarg;
                break;
            case 'f':
                overwrite = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
            case '?':
            default:
                crinitPrintUsage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    crinitClientSetVerbose(verbose);

    char *sockFile = getenv("CRINIT_SOCK");
    if (sockFile != NULL) {
        crinitClientSetSocketPath(sockFile);
    }

    if (ignoreDeps) {
        overDeps = "@empty";
    }

    if (strcmp(getoptArgv[0], "addtask") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (!crinitIsAbsPath(getoptArgv[optind])) {
            crinitErrPrint("The path to the task config to load must be absolute.");
            return EXIT_FAILURE;
        }
        if (crinitClientTaskAdd(getoptArgv[optind], overwrite, overDeps) == -1) {
            crinitErrPrint("Adding task from \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "addseries") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(getoptArgv[0]);
            return EXIT_FAILURE;
        }
        if (!crinitIsAbsPath(getoptArgv[optind])) {
            crinitErrPrint("The path to the series config to load must be absolute.");
            return EXIT_FAILURE;
        }
        if (crinitClientSeriesAdd(getoptArgv[optind], overwrite) == -1) {
            crinitErrPrint("Loading series file \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "enable") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(getoptArgv[0]);
            return EXIT_FAILURE;
        }
        if (crinitClientTaskEnable(getoptArgv[optind]) == -1) {
            crinitErrPrint("Enabling task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "disable") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (crinitClientTaskDisable(getoptArgv[optind]) == -1) {
            crinitErrPrint("Disabling task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "stop") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (crinitClientTaskStop(getoptArgv[optind]) == -1) {
            crinitErrPrint("Stopping task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "kill") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (crinitClientTaskKill(getoptArgv[optind]) == -1) {
            crinitErrPrint("Killing task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "restart") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (crinitClientTaskRestart(getoptArgv[optind]) == -1) {
            crinitErrPrint("Restarting task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "status") == 0) {
        if (getoptArgv[optind] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        crinitTaskState_t s = 0;
        pid_t pid = -1;
        struct timespec ct = {0}, st = {0}, et = {0};
        gid_t gid = 0;
        uid_t uid = 0;
        char ctStr[TIME_REPR_MAX_LEN], stStr[TIME_REPR_MAX_LEN], etStr[TIME_REPR_MAX_LEN];
        const char *state;
        if (crinitClientTaskGetStatus(&s, &pid, &ct, &st, &et, &gid, &uid, getoptArgv[optind]) == -1) {
            crinitErrPrint("Querying status of task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        state = crinitTaskStateToStr(s);
        snprintf(ctStr, sizeof(ctStr), (ct.tv_sec == 0 && ct.tv_nsec == 0) ? "n/a" : TIME_REPR_PRINTF_FORMAT, ct.tv_sec,
                 ct.tv_nsec);
        snprintf(stStr, sizeof(stStr), (st.tv_sec == 0 && st.tv_nsec == 0) ? "n/a" : TIME_REPR_PRINTF_FORMAT, st.tv_sec,
                 st.tv_nsec);
        snprintf(etStr, sizeof(etStr), (et.tv_sec == 0 && et.tv_nsec == 0) ? "n/a" : TIME_REPR_PRINTF_FORMAT, et.tv_sec,
                 et.tv_nsec);
        crinitInfoPrint("Status: %s, PID: %d CTime: %s STime: %s ETime: %s UID: %d GID: %d", state, pid, ctStr, stStr, etStr, uid, gid);
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "notify") == 0) {
        if (getoptArgv[optind] == NULL || argv[optind + 1] == NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        crinitClientSetNotifyTaskName(getoptArgv[optind]);
        if (sd_notify(0, getoptArgv[optind + 1]) == -1) {
            crinitErrPrint("sd_notify() for task \'%s\' with notify-string \'%s\' failed.", getoptArgv[optind],
                           getoptArgv[optind + 1]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(getoptArgv[0], "list") == 0) {
        if (getoptArgv[optind] != NULL) {
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        crinitTaskList_t *tl;
        if (crinitClientGetTaskList(&tl) == -1) {
            crinitErrPrint("Querying list of task \'%s\' failed.", getoptArgv[optind]);
            return EXIT_FAILURE;
        }
        int maxNameLen = 0;
        for (size_t i = 0; i < tl->numTasks; i++) {
            int len = strlen(tl->tasks[i].name);
            if (len > maxNameLen) {
                maxNameLen = len;
            }
        }
        crinitInfoPrint("%-*s  %4s  %5s %5s %s", maxNameLen, "NAME", "PID", "UID", "GID", "STATUS");
        for (size_t i = 0; i < tl->numTasks; i++) {
            const char *state = crinitTaskStateToStr(tl->tasks[i].state);
            crinitInfoPrint("%-*s  %4d  %5d %5d %s", maxNameLen, tl->tasks[i].name, tl->tasks[i].pid, tl->tasks[i].uid, tl->tasks[i].gid, state);
        }
        crinitClientFreeTaskList(tl);
        return EXIT_SUCCESS;
    }
    if (strcmp(basename(getoptArgv[0]), "poweroff") == 0) {
        if (crinitClientShutdown(CRINIT_SHD_POWEROFF) == -1) {
            crinitErrPrint("System poweroff request failed.");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(basename(getoptArgv[0]), "reboot") == 0) {
        if (crinitClientShutdown(CRINIT_SHD_REBOOT) == -1) {
            crinitErrPrint("System reboot request failed.");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    crinitPrintUsage(argv[0]);
    return EXIT_FAILURE;
}

static void crinitPrintUsage(char *prgmPath) {
    if (strcmp(basename(prgmPath), "reboot") == 0) {
        fprintf(stderr,
                "USAGE: %s [-v/--verbose]\n"
                "      Will request Crinit to perform a graceful system reboot.\n"
                "           Specifying '-v/--verbose' will give verbose output.\n",
                prgmPath);
        return;
    }
    if (strcmp(basename(prgmPath), "poweroff") == 0) {
        fprintf(stderr,
                "USAGE: %s [-v/--verbose]\n"
                "      Will request Crinit to perform a graceful system poweroff.\n"
                "           Specifying '-v/--verbose' will give verbose output.\n",
                prgmPath);
        return;
    }
    fprintf(
        stderr,
        "USAGE: %s <ACTION> [OPTIONS] <PARAMETER> [PARAMETERS...]\n"
        "  where ACTION must be exactly one of (including specific options/parameters):\n"
        "     addtask [-f/--overwrite] [-i/--ignore-deps] [-d/--override-deps \"depA:eventA depB:eventB [...]\"] "
        "<PATH>\n"
        "             - Will add a task defined in the task configuration file at <PATH> (absolute) to Crinit's task "
        "database.\n"
        "               \'-f/--overwrite\' - Lets Crinit know it is fine to overwrite if it has already loaded a task\n"
        "                    with the same name.\n"
        "               \'-d/--override-deps <dependency-list>\' - Will override the DEPENDS field of the config file\n"
        "                    with what is given as the parameter.\n"
        "               \'-i/--ignore-deps\' - Shortcut for \'--override-deps \"\"\'.\n"
        "   addseries [-f/--overwrite] <PATH>\n"
        "             - Will load a series file from <PATH>. Options set in the new series file take precedence over\n"
        "               current settings.\n"
        "               \'-f/--overwrite\' - Lets Crinit know it is fine to overwrite if it has already loaded tasks\n"
        "                    with the same name as those in the new series file.\n"
        "      enable <TASK_NAME>\n"
        "             - Removes dependency \'@ctl:enable\' from the dependency list of <TASK_NAME> if it is present.\n"
        "     disable <TASK_NAME>\n"
        "             - Adds dependency \'@ctl:enable\' to the dependency list of <TASK_NAME>.\n"
        "        stop <TASK_NAME>\n"
        "             - If the task has a STOP_COMMAND, it will be executed. Otherwise, Crinit sends SIGTERM to the\n"
        "               PID of <TASK_NAME> if the PID is currently known.\n"
        "        kill <TASK_NAME>\n"
        "             - Sends SIGKILL to the PID of <TASK_NAME> if the PID is currently known.\n"
        "     restart <TASK_NAME>\n"
        "             - Resets the status bits of <TASK_NAME> if it is DONE or FAILED.\n"
        "      status <TASK_NAME>\n"
        "             - Queries status bits, PID, and timestamps of <TASK_NAME>. The CTime, STime, and ETime fields\n"
        "               represent the times the task was Created (loaded/parsed), last Started (became running), and\n"
        "               last Ended (failed or is done). If the event has not occurred yet, the timestamp's value will\n"
        "               be 'n/a'.\n"
        "      notify <TASK_NAME> <\"SD_NOTIFY_STRING\">\n"
        "             - Will send an sd_notify-style status report to Crinit. Only MAINPID and READY are\n"
        "               implemented. See the sd_notify documentation for their meaning.\n"
        "        list\n"
        "             - Print the list of loaded tasks and their status.\n"
        "      reboot\n"
        "             - Will request Crinit to perform a graceful system reboot. crinit-ctl can be symlinked to\n"
        "               reboot as a shortcut which will invoke this command automatically.\n"
        "    poweroff\n"
        "             - Will request Crinit to perform a graceful system shutdown. crinit-ctl can be symlinked to\n"
        "               poweroff as a shortcut which will invoke this command automatically.\n"
        "  General Options:\n"
        "        --verbose/-v - Be verbose.\n"
        "        --help/-h    - Print this help.\n"
        "        --version/-V - Print version information about crinit-ctl, the crinit-client library,\n"
        "                       and -- if connection is successful -- the crinit daemon.\n",
        prgmPath);
}

static void crinitPrintVersion(void) {
    fprintf(stderr, "crinit-ctl version %s\n", crinitGetVersionString());
    const crinitVersion_t *libVer = crinitClientLibGetVersion();
    fprintf(stderr, "crinit-client library version %u.%u.%u%s%s\n", libVer->major, libVer->minor, libVer->micro,
            (strlen(libVer->git) == 0) ? "" : ".", libVer->git);
    crinitVersion_t daemonVer;
    if (crinitClientGetVersion(&daemonVer) == -1) {
        crinitErrPrint("Could not get version of Crinit daemon.");
        return;
    }
    fprintf(stderr, "crinit daemon version %u.%u.%u%s%s\n", daemonVer.major, daemonVer.minor, daemonVer.micro,
            (strlen(daemonVer.git) == 0) ? "" : ".", daemonVer.git);
}

static const char *crinitTaskStateToStr(crinitTaskState_t s) {
    bool notified = s & CRINIT_TASK_STATE_NOTIFIED;
    s &= ~CRINIT_TASK_STATE_NOTIFIED;

    switch (s) {
        case CRINIT_TASK_STATE_LOADED:
            return "loaded";
        case CRINIT_TASK_STATE_STARTING:
            return "starting";
        case CRINIT_TASK_STATE_RUNNING:
            return (notified) ? "running (notified)" : "running";
        case CRINIT_TASK_STATE_DONE:
            return (notified) ? "done (notified)" : "done";
        case CRINIT_TASK_STATE_FAILED:
            return (notified) ? "failed (notified)" : "failed";
        default:
            return "(invalid)";
    }
}
