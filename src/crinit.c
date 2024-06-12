// SPDX-License-Identifier: MIT
/**
 * @file crinit.c
 * @brief Implementation of the Crinit main program.
 */
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "globopt.h"
#include "kcmdline.h"
#include "logio.h"
#include "minsetup.h"
#include "notiserv.h"
#include "optfeat.h"
#include "procdip.h"
#include "rtimcmd.h"
#include "version.h"

#ifdef SIGNATURE_SUPPORT
#include "sig.h"
#endif

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

    if (crinitKernelCmdlineParse(NULL) == -1) {
        crinitErrPrint("Could not parse Kernel cmdline.");
        goto failFreeGlobOpts;
    }

    bool signatures = CRINIT_CONFIG_DEFAULT_SIGNATURES;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_SIGNATURES, &signatures) == -1) {
        crinitErrPrint("Could not retrieve value for global setting if we are to use signatures.");
        goto failFreeGlobOpts;
    }
    if (signatures) {
#ifdef SIGNATURE_SUPPORT
        if (crinitSigSubsysInit(CRINIT_SIGNATURE_DEFAULT_ROOT_KEY_DESC) == -1) {
            crinitErrPrint("Could not initialize signature handling subsystem.");
            goto failFreeGlobOpts;
        }
        char *sigKeyDir;
        if (crinitGlobOptGet(CRINIT_GLOBOPT_SIGKEYDIR, &sigKeyDir) == -1) {
            crinitErrPrint("Could not retrieve path of signature pubkey directory from global options.");
            goto failFreeSigs;
        }

        if (crinitLoadAndVerifySignedKeys(sigKeyDir) == -1) {
            crinitErrPrint("Could not load/verify public keys from '%s'.", sigKeyDir);
            goto failFreeSigs;
        }
#else
        crinitErrPrint(
            "Config signature option is set but signature support was not compiled in. You will need to recompile "
            "Crinit with mbedtls.");
        goto failFreeGlobOpts;
#endif
    }

    if (crinitLoadSeriesConf(seriesFname) == -1) {
        crinitErrPrint("Could not load series file \'%s\'.", seriesFname);
        goto failFreeSigs;
    }

    // Initialize optional features as soon as possible.
    crinitInfoPrint("Initialize optional features.");
    if (crinitFeatureHook(NULL, CRINIT_HOOK_INIT, NULL) == -1) {
        crinitErrPrint("Could not run activation hook for feature \'INIT\'.");
        goto failFreeSigs;
    }

    crinitFileSeries_t taskSeries;
    if (crinitLoadTasks(&taskSeries) == -1) {
        crinitErrPrint("Could not load crinit task.");
        goto failFreeSigs;
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
                crinitDestroyFileSeries(&taskSeries);
                goto failFreeTaskDB;
            }
            memcpy(confFn, taskSeries.baseDir, prefixLen);
            confFn[prefixLen] = '/';
            memcpy(confFn + prefixLen + 1, taskSeries.fnames[n], suffixLen + 1);
            confFnAllocated = true;
        }
        crinitConfKvList_t *c;
        if (crinitParseConf(&c, confFn) == -1) {
            crinitErrPrint("Could not parse file \'%s\'.", confFn);
            if (confFnAllocated) {
                free(confFn);
            }
            crinitDestroyFileSeries(&taskSeries);
            goto failFreeTaskDB;
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
            crinitDestroyFileSeries(&taskSeries);
            goto failFreeTaskDB;
        }
        crinitFreeConfList(c);

        crinitDbgInfoPrint("Task extracted without error.");
        crinitTaskPrint(t);

        if (crinitTaskDBInsert(&tdb, t, false) == -1) {
            crinitErrPrint("Could not insert Task '%s' into TaskDB.", t->name);
            crinitDestroyFileSeries(&taskSeries);
            crinitFreeTask(t);
            goto failFreeTaskDB;
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
        goto failFreeTaskDB;
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
#ifdef SIGNATURE_SUPPORT
    if (signatures) {
        crinitSigSubsysDestroy();
    }
#endif
    return EXIT_SUCCESS;

failFreeTaskDB:
    crinitTaskDBDestroy(&tdb);
failFreeSigs:
#ifdef SIGNATURE_SUPPORT
    if (signatures) {
        crinitSigSubsysDestroy();
    }
#endif
failFreeGlobOpts:
    crinitGlobOptDestroy();
    return EXIT_FAILURE;
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
