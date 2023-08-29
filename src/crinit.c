// SPDX-License-Identifier: MIT
/**
 * @file crinit.c
 * @brief Implementation of the Crinit main program.
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
#define CRINIT_DEFAULT_CONFIG_SERIES "/etc/crinit/default.series"

/**
 * Prints a message indicating Crinit's version to stderr.
 */
static void crinitPrintVersion(void);
/**
 * Print usage information for Crinit to stderr.
 *
 * Includes version message via crinitPrintVersion().
 *
 * @param basename  The name of this executable, according to argv[0].
 */
static void crinitPrintUsage(const char *basename);
/**
 * Print out the contents of an crinitTask_t structure in a readable format using crinitDbgInfoPrint().
 *
 * @param t  The task to be printed.
 */
static void crinitTaskPrint(const crinitTask_t *t);

/**
 * Main function of crinit.
 *
 * Will perform minimal system setup, fork from PID 1 (which remains as a zombie reaper process), construct a TaskDB
 * from the given configuration and then spawn tasks as they are ready.
 */
int main(int argc, char *argv[]) {
    const char *seriesFname = CRINIT_DEFAULT_CONFIG_SERIES;
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (crinitParamCheck(argv[i], "-V", "--version")) {
                crinitPrintVersion();
                return EXIT_FAILURE;
            }
            if (crinitParamCheck(argv[i], "-h", "--help")) {
                crinitPrintUsage(argv[0]);
                return EXIT_FAILURE;
            }
        }
        if (!crinitIsAbsPath(argv[1])) {
            crinitErrPrint("Program argument must be an absolute path.");
            crinitPrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
        seriesFname = argv[1];
    }
    crinitInfoPrint("Crinit daemon version %s started.", crinitGetVersionString());
    if (getpid() == 1) {
        if (crinitForkZombieReaper() == -1) {
            crinitErrPrint("I am PID 1 but failed to create a zombie reaper process.");
            return EXIT_FAILURE;
        }
        if (crinitSetupSystemFs() == -1) {
            crinitErrPrint("I started as PID 1 but failed to do minimal system setup.");
            return EXIT_FAILURE;
        }
    }
    if (crinitGlobOptInitDefault() == -1) {
        crinitErrPrint("Could not initialize global option array.");
        return EXIT_FAILURE;
    }

    crinitFileSeries_t taskSeries;
    if (crinitLoadSeriesConf(&taskSeries, seriesFname) == -1) {
        crinitErrPrint("Could not load series file \'%s\'.", seriesFname);
        crinitGlobOptDestroy();
        return EXIT_FAILURE;
    }

    // TODO: Init features (call feature hook init).
    crinitRtimOpMapDebugPrintAll();

    crinitTaskDB_t tdb;
    crinitTaskDBInit(&tdb, crinitProcDispatchSpawnFunc);

    for (size_t n = 0; n < taskSeries.size; n++) {
        char *confFn = taskSeries.fnames[n];
        bool confFnAllocated = false;
        if (!crinitIsAbsPath(confFn)) {
            size_t prefixLen = strlen(taskSeries.baseDir);
            size_t suffixLen = strlen(taskSeries.fnames[n]);
            confFn = malloc(prefixLen + suffixLen + 2);
            if (confFn == NULL) {
                crinitErrnoPrint("Could not allocate string with full path for \'%s\'.", taskSeries.fnames[n]);
                crinitGlobOptDestroy();
                crinitDestroyFileSeries(&taskSeries);
                crinitTaskDBDestroy(&tdb);
                return EXIT_FAILURE;
            }
            memcpy(confFn, taskSeries.baseDir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, taskSeries.fnames[n], suffixLen + 1);
            confFnAllocated = true;
        }
        crinitConfKvList_t *c;
        if (crinitParseConf(&c, confFn) == -1) {
            crinitErrPrint("Could not parse file \'%s\'.", confFn);
            crinitGlobOptDestroy();
            if (confFnAllocated) {
                free(confFn);
            }
            crinitDestroyFileSeries(&taskSeries);
            crinitTaskDBDestroy(&tdb);
            return EXIT_FAILURE;
        }
        crinitInfoPrint("File \'%s\' loaded.", confFn);
        if (confFnAllocated) {
            free(confFn);
        }
        crinitDbgInfoPrint("Will now attempt to extract a Task out of the config.");

        crinitTask_t *t = NULL;
        if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
            crinitErrPrint("Could not extract task from ConfKvList.");
            crinitFreeConfList(c);
            crinitGlobOptDestroy();
            crinitDestroyFileSeries(&taskSeries);
            crinitTaskDBDestroy(&tdb);
            return EXIT_FAILURE;
        }
        crinitFreeConfList(c);

        crinitDbgInfoPrint("Task extracted without error.");
        crinitTaskPrint(t);

        if (crinitTaskDBInsert(&tdb, t, false) == -1) {
            crinitErrPrint("Could not insert Task '%s' into TaskDB.", t->name);
            crinitGlobOptDestroy();
            crinitDestroyFileSeries(&taskSeries);
            crinitFreeTask(t);
            crinitTaskDBDestroy(&tdb);
            return EXIT_FAILURE;
        }
        crinitFreeTask(t);
    }
    crinitDestroyFileSeries(&taskSeries);
    crinitDbgInfoPrint("Done parsing.");

    char *sockFile = getenv("CRINIT_SOCK");
    if (sockFile == NULL) {
        sockFile = CRINIT_SOCKFILE;
    }
    if (crinitStartInterfaceServer(&tdb, sockFile) == -1) {
        crinitErrPrint("Could not start notification and service interface.");
        crinitTaskDBDestroy(&tdb);
        crinitGlobOptDestroy();
        return EXIT_FAILURE;
    }

    while (true) {
        crinitTaskDBSpawnReady(&tdb);
        pthread_mutex_lock(&tdb.lock);
        crinitDbgInfoPrint("Waiting for Task to be ready.");
        pthread_cond_wait(&tdb.changed, &tdb.lock);
        pthread_mutex_unlock(&tdb.lock);
    }
    crinitTaskDBDestroy(&tdb);
    crinitGlobOptDestroy();
    return EXIT_SUCCESS;
}

static void crinitPrintVersion(void) {
    fprintf(stderr, "Crinit version %s\n", crinitGetVersionString());
}

static void crinitPrintUsage(const char *basename) {
    crinitPrintVersion();
    fprintf(stderr, "USAGE: %s [path/to/config.series]\n", basename);
    fprintf(stderr, "If nothing is specified, the default path \'%s\' is used.\n", CRINIT_DEFAULT_CONFIG_SERIES);
}

static void crinitTaskPrint(const crinitTask_t *t) {
    crinitDbgInfoPrint("---------------");
    crinitDbgInfoPrint("Data Structure:");
    crinitDbgInfoPrint("---------------");
    crinitDbgInfoPrint("NAME: %s", t->name);
    crinitDbgInfoPrint("Number of COMMANDs: %zu", t->cmdsSize);
    for (size_t i = 0; i < t->cmdsSize; i++) {
        crinitDbgInfoPrint("cmds[%zu]:", i);
        for (int j = 0; j <= t->cmds[i].argc; j++) {
            if (t->cmds[i].argv[j] != NULL) {
                crinitDbgInfoPrint("    argv[%d] = \'%s\'", j, t->cmds[i].argv[j]);
            } else {
                crinitDbgInfoPrint("    argv[%d] = NULL", j);
            }
        }
    }

    crinitDbgInfoPrint("Number of dependencies: %zu", t->depsSize);
    for (size_t i = 0; i < t->depsSize; i++) {
        crinitDbgInfoPrint("deps[%zu]: name=\'%s\' event=\'%s\'", i, t->deps[i].name, t->deps[i].event);
    }

    crinitDbgInfoPrint("Number of provided features: %zu", t->prvSize);
    for (size_t i = 0; i < t->prvSize; i++) {
        crinitDbgInfoPrint("prv[%zu]: name=\'%s\' state_req=\'%lu\'", i, t->prv[i].name, t->prv[i].stateReq);
    }

    crinitDbgInfoPrint("TaskOpts:");
    crinitDbgInfoPrint("    CRINIT_TASK_OPT_RESPAWN = %s", (t->opts & CRINIT_TASK_OPT_RESPAWN) ? "true" : "false");
}
