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
#include "crinit.h"

#include <stdlib.h>
#include <unistd.h>

#include "globopt.h"
#include "logio.h"
#include "minsetup.h"
#include "procdip.h"
/**
 * Print usage information for Crinit.
 *
 * @param basename  The name of this executable, according to argv[0].
 */
static void printUsage(const char *basename);
/**
 * Check if \a path is absolute (i.e. starts with '/').
 *
 * @param path  The path to check.
 *
 * @return true if path is absolute, false otherwise
 */
static bool isAbsPath(const char *path);
/**
 * Parse a series file.
 *
 * Will return the task config files to be loaded in \a series. Will also set any global options specified in the series
 * file.
 *
 * @param seriesLen  Returns how many task configs there are in \a series.
 * @param series     Returns the paths to the task configs specified in the series file.
 * @param filename   The path to the series file to load.
 *
 * @return 0 on success, -1 on failure
 */
static int loadSeriesConf(int *seriesLen, char ***series, const char *filename);
/**
 * Print out the contents of an ebcl_Task structure in a readable format using EBCL_dbgInfoPrint().
 *
 * @param t  The task to be printed.
 */
static void taskPrint(const ebcl_Task *t);

/**
 * Main function of crinit.
 *
 * Will perform minimal system setup, fork from PID 1 (which remains as a zombie reaper process), construct a TaskDB
 * from the given configuration and then spawn tasks as they are ready.
 */
int main(int argc, char *argv[]) {
    const char *seriesFname = EBCL_CRINIT_DEFAULT_CONFIG_SERIES;
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        if (!isAbsPath(argv[1])) {
            EBCL_errPrint("Program argument must be an absolute path.");
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        seriesFname = argv[1];
    }
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
    if (loadSeriesConf(&seriesLen, &series, seriesFname) == -1) {
        EBCL_errPrint("Could not load series file \'%s\'.", seriesFname);
        EBCL_globOptDestroy();
        return EXIT_FAILURE;
    }

    ebcl_TaskDB tdb;
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
        if (!isAbsPath(confFn)) {
            size_t prefixLen = strlen(taskdir);
            size_t suffixLen = strlen(series[n]);
            confFn = malloc(prefixLen + suffixLen + 2);
            if (confFn == NULL) {
                EBCL_errnoPrint("Could not allocate string with full path for \'%s\'.", series[n]);
                EBCL_globOptDestroy();
                EBCL_freeArgvArray(series);
                return EXIT_FAILURE;
            }
            memcpy(confFn, taskdir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, series[n], suffixLen + 1);
            confFnAllocated = true;
        }
        ebcl_ConfKvList *c;
        if (EBCL_parseConf(&c, confFn) == -1) {
            EBCL_errPrint("Could not parse file \'%s\'.", confFn);
            EBCL_globOptDestroy();
            if (confFnAllocated) {
                free(confFn);
            }
            EBCL_freeArgvArray(series);
            return EXIT_FAILURE;
        }
        EBCL_infoPrint("File \'%s\' loaded.", confFn);
        if (confFnAllocated) {
            free(confFn);
        }
        EBCL_dbgInfoPrint("Will now attempt to extract a Task out of the config.");

        ebcl_Task *t = NULL;
        if (EBCL_taskCreateFromConfKvList(&t, c) == -1) {
            EBCL_errPrint("Could not extract task from ConfKvList.");
            EBCL_freeConfList(c);
            EBCL_globOptDestroy();
            EBCL_freeArgvArray(series);
            return EXIT_FAILURE;
        }
        EBCL_freeConfList(c);

        EBCL_dbgInfoPrint("Task extracted without error.");
        taskPrint(t);

        EBCL_taskDBInsert(&tdb, t, false);
        EBCL_freeTask(t);
    }
    EBCL_freeArgvArray(series);
    EBCL_dbgInfoPrint("Done parsing.");

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

static void printUsage(const char *basename) {
    fprintf(stderr, "USAGE: %s [path/to/config.series]\n", basename);
    fprintf(stderr, "If nothing is specified, the default path \'%s\' is used.\n", EBCL_CRINIT_DEFAULT_CONFIG_SERIES);
}

static bool isAbsPath(const char *path) {
    if (path == NULL) return false;
    return (path[0] == '/');
}

static int loadSeriesConf(int *seriesLen, char ***series, const char *filename) {
    if (seriesLen == NULL || series == NULL || !isAbsPath(filename)) {
        EBCL_errPrint("Parameters must not be NULL and filename must be an absolute path.");
        return -1;
    }
    ebcl_ConfKvList *c;
    if (EBCL_parseConf(&c, filename) == -1) {
        EBCL_errPrint("Could not parse file \'%s\'.", filename);
        return -1;
    }
    if (EBCL_confListExtractArgvArray(seriesLen, series, "TASKS", c, true) == -1) {
        EBCL_errPrint("Could not extract value for key \'TASKS\' from \'%s\'.", filename);
        return -1;
    }

    bool confDbg = false;
    if (EBCL_confListExtractBoolean(&confDbg, EBCL_GLOBOPT_KEYSTR_DEBUG, c) == 0) {
        if (EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &confDbg) == -1) {
            EBCL_errPrint("Could not store global boolean option value for \'%s\'.", EBCL_GLOBOPT_KEYSTR_DEBUG);
            EBCL_freeConfList(c);
            EBCL_freeArgvArray(*series);
            return -1;
        }
    }

    bool confFsigs = false;
    if (EBCL_confListExtractBoolean(&confDbg, EBCL_GLOBOPT_KEYSTR_FILESIGS, c) == 0) {
        if (EBCL_globOptSetBoolean(EBCL_GLOBOPT_FILESIGS, &confFsigs) == -1) {
            EBCL_errPrint("Could not store global boolean option value for \'%s\'.", EBCL_GLOBOPT_KEYSTR_FILESIGS);
            EBCL_freeConfList(c);
            EBCL_freeArgvArray(*series);
            return -1;
        }
    }

    char *taskDir = NULL;
    if (EBCL_confListGetVal(&taskDir, EBCL_GLOBOPT_KEYSTR_TASKDIR, c) == 0) {
        if (EBCL_globOptSetString(EBCL_GLOBOPT_TASKDIR, taskDir) == -1) {
            EBCL_errPrint("Could not store global string option values for \'%s\'.", EBCL_GLOBOPT_KEYSTR_TASKDIR);
            EBCL_freeConfList(c);
            EBCL_freeArgvArray(*series);
            return -1;
        }
    }

    EBCL_freeConfList(c);
    return 0;
}

static void taskPrint(const ebcl_Task *t) {
    EBCL_dbgInfoPrint("---------------");
    EBCL_dbgInfoPrint("Data Structure:");
    EBCL_dbgInfoPrint("---------------");
    EBCL_dbgInfoPrint("NAME: %s", t->name);
    EBCL_dbgInfoPrint("Number of COMMANDs: %lu", t->cmdsSize);
    for (size_t i = 0; i < t->cmdsSize; i++) {
        EBCL_dbgInfoPrint("cmds[%lu]:", i);
        for (int j = 0; j <= t->cmds[i].argc; j++) {
            if (t->cmds[i].argv[j] != NULL) {
                EBCL_dbgInfoPrint("    argv[%d] = \'%s\'", j, t->cmds[i].argv[j]);
            } else {
                EBCL_dbgInfoPrint("    argv[%d] = NULL", j);
            }
        }
    }

    EBCL_dbgInfoPrint("Number of dependencies: %lu", t->depsSize);
    for (size_t i = 0; i < t->depsSize; i++) {
        EBCL_dbgInfoPrint("deps[%lu]: name=\'%s\' event=\'%s\'", i, t->deps[i].name, t->deps[i].event);
    }

    EBCL_dbgInfoPrint("TaskOpts:");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_EXEC    = %s", (t->opts & EBCL_TASK_OPT_EXEC) ? "true" : "false");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_QM_JAIL = %s", (t->opts & EBCL_TASK_OPT_QM_JAIL) ? "true" : "false");
    EBCL_dbgInfoPrint("    EBCL_TASK_OPT_RESPAWN = %s", (t->opts & EBCL_TASK_OPT_RESPAWN) ? "true" : "false");
}

