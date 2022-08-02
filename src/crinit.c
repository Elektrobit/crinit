/**
 * @file crinit.c
 * @brief Implementation of the Crinit main program.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "globopt.h"
#include "logio.h"
#include "minsetup.h"
#include "notiserv.h"
#include "procdip.h"
#include "rtimcmd.h"
#include "version.h"

/**
 * The default series file. Used if nothing is specified on command line.
 */
#define EBCL_CRINIT_DEFAULT_CONFIG_SERIES "/etc/crinit/default.series"

/**
 * Prints a message indicating Crinit's version to stderr.
 */
static void EBCL_printVersion(void);
/**
 * Print usage information for Crinit to stderr.
 *
 * Includes version message via EBCL_printVersion().
 *
 * @param basename  The name of this executable, according to argv[0].
 */
static void EBCL_printUsage(const char *basename);
/**
 * Check if \a path is absolute (i.e. starts with '/').
 *
 * @param path  The path to check.
 *
 * @return true if path is absolute, false otherwise
 */
static bool EBCL_isAbsPath(const char *path);
/**
 * Print out the contents of an ebcl_Task_t structure in a readable format using EBCL_dbgInfoPrint().
 *
 * @param t  The task to be printed.
 */
static void EBCL_taskPrint(const ebcl_Task_t *t);

/**
 * Main function of crinit.
 *
 * Will perform minimal system setup, fork from PID 1 (which remains as a zombie reaper process), construct a TaskDB
 * from the given configuration and then spawn tasks as they are ready.
 */
int main(int argc, char *argv[]) {
    const char *seriesFname = EBCL_CRINIT_DEFAULT_CONFIG_SERIES;
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (EBCL_paramCheck(argv[i], "-V", "--version")) {
                EBCL_printVersion();
                return EXIT_FAILURE;
            }
            if (EBCL_paramCheck(argv[i], "-h", "--help")) {
                EBCL_printUsage(argv[0]);
                return EXIT_FAILURE;
            }
        }
        if (!EBCL_isAbsPath(argv[1])) {
            EBCL_errPrint("Program argument must be an absolute path.");
            EBCL_printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        seriesFname = argv[1];
    }
    EBCL_infoPrint("Crinit daemon version %s started.", EBCL_getVersionString());
    if (getpid() == 1) {
        if (EBCL_forkZombieReaper() == -1) {
            EBCL_errPrint("I am PID 1 but failed to create a zombie reaper process.");
            return EXIT_FAILURE;
        }
        if (EBCL_setupSystemFs() == -1) {
            EBCL_errPrint("I started as PID 1 but failed to do minimal system setup.");
            return EXIT_FAILURE;
        }
    }
    if (EBCL_globOptInitDefault() == -1) {
        EBCL_errPrint("Could not initialize global option array.");
        return EXIT_FAILURE;
    }

    char **series = NULL;
    int seriesLen = 0;
    if (EBCL_loadSeriesConf(&seriesLen, &series, seriesFname) == -1) {
        EBCL_errPrint("Could not load series file \'%s\'.", seriesFname);
        EBCL_globOptDestroy();
        return EXIT_FAILURE;
    }

    EBCL_rtimOpMapDebugPrintAll();

    ebcl_TaskDB_t tdb;
    EBCL_taskDBInit(&tdb, EBCL_procDispatchSpawnFunc);

    char *taskdir;
    if (EBCL_globOptGetString(EBCL_GLOBOPT_TASKDIR, &taskdir) == -1) {
        EBCL_errPrint("Could not get value for \'TASKDIR\' from global options.");
        EBCL_globOptDestroy();
        return EXIT_FAILURE;
    }

    for (int n = 0; n < seriesLen; n++) {
        char *confFn = series[n];
        bool confFnAllocated = false;
        if (!EBCL_isAbsPath(confFn)) {
            size_t prefixLen = strlen(taskdir);
            size_t suffixLen = strlen(series[n]);
            confFn = malloc(prefixLen + suffixLen + 2);
            if (confFn == NULL) {
                EBCL_errnoPrint("Could not allocate string with full path for \'%s\'.", series[n]);
                EBCL_globOptDestroy();
                EBCL_freeArgvArray(series);
                free(taskdir);
                return EXIT_FAILURE;
            }
            memcpy(confFn, taskdir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, series[n], suffixLen + 1);
            confFnAllocated = true;
        }
        ebcl_ConfKvList_t *c;
        if (EBCL_parseConf(&c, confFn) == -1) {
            EBCL_errPrint("Could not parse file \'%s\'.", confFn);
            EBCL_globOptDestroy();
            if (confFnAllocated) {
                free(confFn);
            }
            EBCL_freeArgvArray(series);
            free(taskdir);
            return EXIT_FAILURE;
        }
        EBCL_infoPrint("File \'%s\' loaded.", confFn);
        if (confFnAllocated) {
            free(confFn);
        }
        EBCL_dbgInfoPrint("Will now attempt to extract a Task out of the config.");

        ebcl_Task_t *t = NULL;
        if (EBCL_taskCreateFromConfKvList(&t, c) == -1) {
            EBCL_errPrint("Could not extract task from ConfKvList.");
            EBCL_freeConfList(c);
            EBCL_globOptDestroy();
            EBCL_freeArgvArray(series);
            free(taskdir);
            return EXIT_FAILURE;
        }
        EBCL_freeConfList(c);

        EBCL_dbgInfoPrint("Task extracted without error.");
        EBCL_taskPrint(t);

        EBCL_taskDBInsert(&tdb, t, false);
        EBCL_freeTask(t);
    }
    EBCL_freeArgvArray(series);
    free(taskdir);
    EBCL_dbgInfoPrint("Done parsing.");

    char *sockFile = getenv("CRINIT_SOCK");
    if (sockFile == NULL) {
        sockFile = EBCL_CRINIT_SOCKFILE;
    }
    if (EBCL_startInterfaceServer(&tdb, sockFile) == -1) {
        EBCL_errPrint("Could not start notification and service interface.");
        EBCL_taskDBDestroy(&tdb);
        EBCL_globOptDestroy();
        return EXIT_FAILURE;
    }

    while (true) {
        EBCL_taskDBSpawnReady(&tdb);
        pthread_mutex_lock(&tdb.lock);
        EBCL_dbgInfoPrint("Waiting for Task to be ready.");
        pthread_cond_wait(&tdb.changed, &tdb.lock);
        pthread_mutex_unlock(&tdb.lock);
    }
    EBCL_taskDBDestroy(&tdb);
    EBCL_globOptDestroy();
    return EXIT_SUCCESS;
}

static void EBCL_printVersion(void) {
    fprintf(stderr, "Crinit version %s\n", EBCL_getVersionString());
}

static void EBCL_printUsage(const char *basename) {
    EBCL_printVersion();
    fprintf(stderr, "USAGE: %s [path/to/config.series]\n", basename);
    fprintf(stderr, "If nothing is specified, the default path \'%s\' is used.\n", EBCL_CRINIT_DEFAULT_CONFIG_SERIES);
}

static bool EBCL_isAbsPath(const char *path) {
    if (path == NULL) return false;
    return (path[0] == '/');
}

static void EBCL_taskPrint(const ebcl_Task_t *t) {
    EBCL_dbgInfoPrint("---------------");
    EBCL_dbgInfoPrint("Data Structure:");
    EBCL_dbgInfoPrint("---------------");
    EBCL_dbgInfoPrint("NAME: %s", t->name);
    EBCL_dbgInfoPrint("Number of COMMANDs: %zu", t->cmdsSize);
    for (size_t i = 0; i < t->cmdsSize; i++) {
        EBCL_dbgInfoPrint("cmds[%zu]:", i);
        for (int j = 0; j <= t->cmds[i].argc; j++) {
            if (t->cmds[i].argv[j] != NULL) {
                EBCL_dbgInfoPrint("    argv[%d] = \'%s\'", j, t->cmds[i].argv[j]);
            } else {
                EBCL_dbgInfoPrint("    argv[%d] = NULL", j);
            }
        }
    }

    EBCL_dbgInfoPrint("Number of dependencies: %zu", t->depsSize);
    for (size_t i = 0; i < t->depsSize; i++) {
        EBCL_dbgInfoPrint("deps[%zu]: name=\'%s\' event=\'%s\'", i, t->deps[i].name, t->deps[i].event);
    }

    EBCL_dbgInfoPrint("TaskOpts:");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_EXEC    = %s", (t->opts & EBCL_TASK_OPT_EXEC) ? "true" : "false");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_QM_JAIL = %s", (t->opts & EBCL_TASK_OPT_QM_JAIL) ? "true" : "false");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_RESPAWN = %s", (t->opts & EBCL_TASK_OPT_RESPAWN) ? "true" : "false");
}

