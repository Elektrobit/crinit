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
 *     enable <TASK_NAME>
 *            - Removes dependency '@ctl:enable' from the dependency list of <TASK_NAME> if it is present.
 *    disable <TASK_NAME>
 *            - Adds dependency '@ctl:enable' to the dependency list of <TASK_NAME>.
 *       stop <TASK_NAME>
 *            - Sends SIGTERM to the PID of <TASK_NAME> if the PID is currently known.
 *       kill <TASK_NAME>
 *            - Sends SIGKILL to the PID of <TASK_NAME> if the PID is currently known.
 *    restart <TASK_NAME>
 *            - Resets the status bits of <TASK_NAME> if it is DONE or FAILED.
 *     status <TASK_NAME>
 *            - Queries status bits and PID of <TASK_NAME>.
 *     notify <TASK_NAME> <"SD_NOTIFY_STRING">
 *            - Will send an sd_notify-style status report to Crinit. Only MAINPID and READY are
 *              implemented. See the sd_notify documentation for their meaning.
 * General Options:
 *       --verbose/-v - Be verbose.
 *       --help/-h    - Print this help.
 * ~~~
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "crinit-client.h"
#include "logio.h"

/**
 * Check if \a path is absolute (i.e. starts with '/').
 *
 * @param path  The path to check.
 *
 * @return true if path is absolute, false otherwise
 */
static bool isAbsPath(const char *path);
/**
 * Print usage information.
 *
 * @param basename  The name of the program, usually found in argv[0].
 */
static void printUsage(const char *basename);

int main(int argc, char *argv[]) {
    EBCL_setPrintPrefix("");

    if (argc < 3) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
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
        int optionIndex = 0;
        opt = getopt_long(argc - 1, argv + 1, "hd:fiv", longOptions, &optionIndex);
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
                printUsage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    optind += 1;

    EBCL_crinitSetVerbose(verbose);

    if (ignoreDeps) {
        overDeps = "@empty";
    }
    if (strcmp(argv[1], "addtask") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (!isAbsPath(argv[optind])) {
            EBCL_errPrint("The path to the task config to load must be absolute.");
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskAdd(argv[optind], overwrite, overDeps) == -1) {
            EBCL_errPrint("Adding task from \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "enable") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskEnable(argv[optind]) == -1) {
            EBCL_errPrint("Enabling task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "disable") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskDisable(argv[optind]) == -1) {
            EBCL_errPrint("Disabling task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "stop") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskStop(argv[optind]) == -1) {
            EBCL_errPrint("Stopping task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "kill") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskKill(argv[optind]) == -1) {
            EBCL_errPrint("Killing task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "restart") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (EBCL_crinitTaskRestart(argv[optind]) == -1) {
            EBCL_errPrint("Restarting task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "status") == 0) {
        if (argv[optind] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        ebcl_TaskState s = 0;
        pid_t pid = -1;
        if (EBCL_crinitTaskGetStatus(&s, &pid, argv[optind]) == -1) {
            EBCL_errPrint("Querying status of task \'%s\' failed.", argv[optind]);
            return EXIT_FAILURE;
        }
        EBCL_infoPrint("Status: %d, PID: %d", s, pid);
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "notify") == 0) {
        if (argv[optind] == NULL || argv[optind + 1] == NULL) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        EBCL_crinitSetNotifyTaskName(argv[optind]);
        if (sd_notify(0, argv[optind + 1]) == -1) {
            EBCL_errPrint("sd_notify() for task \'%s\' with notify-string \'%s\' failed.", argv[optind],
                          argv[optind + 1]);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    printUsage(argv[0]);
    return EXIT_FAILURE;
}

static void printUsage(const char *basename) {
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
        "      enable <TASK_NAME>\n"
        "             - Removes dependency \'@ctl:enable\' from the dependency list of <TASK_NAME> if it is present.\n"
        "     disable <TASK_NAME>\n"
        "             - Adds dependency \'@ctl:enable\' to the dependency list of <TASK_NAME>.\n"
        "        stop <TASK_NAME>\n"
        "             - Sends SIGTERM to the PID of <TASK_NAME> if the PID is currently known.\n"
        "        kill <TASK_NAME>\n"
        "             - Sends SIGKILL to the PID of <TASK_NAME> if the PID is currently known.\n"
        "     restart <TASK_NAME>\n"
        "             - Resets the status bits of <TASK_NAME> if it is DONE or FAILED.\n"
        "      status <TASK_NAME>\n"
        "             - Queries status bits and PID of <TASK_NAME>.\n"
        "      notify <TASK_NAME> <\"SD_NOTIFY_STRING\">\n"
        "             - Will send an sd_notify-style status report to Crinit. Only MAINPID and READY are\n"
        "               implemented. See the sd_notify documentation for their meaning.\n"
        "  General Options:\n"
        "        --verbose/-v - Be verbose.\n"
        "        --help/-h    - Print this help.\n",
        basename);
}

static bool isAbsPath(const char *path) {
    if (path == NULL) return false;
    return (path[0] == '/');
}

