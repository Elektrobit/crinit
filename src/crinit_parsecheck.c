/**
 * @file crinit_parsecheck.c
 * @brief Implementation of a simple program to test file parsing.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <stdlib.h>
#include <string.h>

#include "logio.h"
#include "taskdb.h"

/**
 * Print out the contents of an crinitTask_t structure in a readable format using crinitInfoPrint().
 *
 * @param t  The task to be printed.
 */
static void crinitTaskPrint(const crinitTask_t *t) {
    crinitInfoPrint("---------------");
    crinitInfoPrint("Data Structure:");
    crinitInfoPrint("---------------");
    crinitInfoPrint("NAME: %s", t->name);
    crinitInfoPrint("Number of COMMANDs: %zu", t->cmdsSize);
    for (size_t i = 0; i < t->cmdsSize; i++) {
        crinitInfoPrint("cmds[%zu]:", i);
        for (int j = 0; j <= t->cmds[i].argc; j++) {
            if (t->cmds[i].argv[j] != NULL) {
                crinitInfoPrint("    argv[%d] = \'%s\'", j, t->cmds[i].argv[j]);
            } else {
                crinitInfoPrint("    argv[%d] = NULL", j);
            }
        }
    }

    crinitInfoPrint("Number of dependencies: %zu", t->depsSize);
    for (size_t i = 0; i < t->depsSize; i++) {
        crinitInfoPrint("deps[%zu]: name=\'%s\' event=\'%s\'", i, t->deps[i].name, t->deps[i].event);
    }

    crinitInfoPrint("TaskOpts:");
    crinitInfoPrint("    CRINIT_TASK_OPT_RESPAWN = %s", (t->opts & CRINIT_TASK_OPT_RESPAWN) ? "true" : "false");
}

/**
 * Main function of crinit_parsecheck.
 *
 * Will try to parse and print out all task configurations given on the command line.
 */
int main(int argc, char *argv[]) {
    for (int n = 1; n < argc; n++) {
        crinitConfKvList_t *c;
        if (crinitParseConf(&c, argv[n]) == -1) {
            crinitErrPrint("Could not parse file \'%s\'.", argv[n]);
            return EXIT_FAILURE;
        }
        crinitInfoPrint("File \'%s\' loaded successfully.", argv[n]);
        crinitInfoPrint("---------");
        crinitInfoPrint("Contents:");
        crinitInfoPrint("---------");
        crinitConfKvList_t *runner = c;
        do {
            if (runner->key != NULL && runner->val != NULL) {
                if (strncmp(runner->key, "COMMAND", 7) == 0) {
                    char **argArr = NULL;
                    int argCount = 0;
                    crinitInfoPrint("\'%s\':", runner->key);
                    if (crinitConfListExtractArgvArray(&argCount, &argArr, runner->key, true, c, true) == -1) {
                        crinitErrPrint("Could not get argv-array for key \'%s\'.", runner->key);
                        crinitFreeConfList(c);
                        return EXIT_FAILURE;
                    }
                    for (int i = 0; i < argCount; i++) {
                        crinitInfoPrint("    ARGV[%d] = \'%s\'", i, argArr[i]);
                    }
                    crinitFreeArgvArray(argArr);
                } else {
                    crinitInfoPrint("\'%s\'=\'%s\'", runner->key, runner->val);
                }
            }
            runner = runner->next;
        } while (runner != NULL);
        crinitInfoPrint(CRINIT_PRINT_EMPTY_LINE);
        crinitInfoPrint("Will now attempt to extract a Task out of the config.");
        crinitTask_t *t = NULL;
        if (crinitTaskCreateFromConfKvList(&t, c) == -1) {
            crinitErrPrint("Could not extract task from ConfKvList.");
            crinitFreeConfList(c);
            return EXIT_FAILURE;
        }
        crinitFreeConfList(c);

        crinitInfoPrint("Task extracted without error.");
        crinitTaskPrint(t);

        crinitInfoPrint("Will now attempt to duplicate the task and print out its (hopefully equal) contents.");
        crinitTask_t *u = NULL;
        if (crinitTaskDup(&u, t) == -1) {
            crinitErrPrint("Could not duplicate the task.");
            return EXIT_FAILURE;
        }
        crinitFreeTask(t);
        crinitTaskPrint(u);
        crinitFreeTask(u);
    }
    crinitInfoPrint("Done.");
    return EXIT_SUCCESS;
}
