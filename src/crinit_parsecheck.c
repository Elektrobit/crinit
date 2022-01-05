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
 * Print out the contents of an ebcl_Task_t structure in a readable format using EBCL_infoPrint().
 *
 * @param t  The task to be printed.
 */
static void EBCL_taskPrint(const ebcl_Task_t *t) {
    EBCL_infoPrint("---------------");
    EBCL_infoPrint("Data Structure:");
    EBCL_infoPrint("---------------");
    EBCL_infoPrint("NAME: %s", t->name);
    EBCL_infoPrint("Number of COMMANDs: %zu", t->cmdsSize);
    for (size_t i = 0; i < t->cmdsSize; i++) {
        EBCL_infoPrint("cmds[%zu]:", i);
        for (int j = 0; j <= t->cmds[i].argc; j++) {
            if (t->cmds[i].argv[j] != NULL) {
                EBCL_infoPrint("    argv[%d] = \'%s\'", j, t->cmds[i].argv[j]);
            } else {
                EBCL_infoPrint("    argv[%d] = NULL", j);
            }
        }
    }

    EBCL_infoPrint("Number of dependencies: %zu", t->depsSize);
    for (size_t i = 0; i < t->depsSize; i++) {
        EBCL_infoPrint("deps[%zu]: name=\'%s\' event=\'%s\'", i, t->deps[i].name, t->deps[i].event);
    }

    EBCL_infoPrint("TaskOpts:");
    EBCL_infoPrint("    EBCL_TASK_OPT_EXEC    = %s", (t->opts & EBCL_TASK_OPT_EXEC) ? "true" : "false");
    EBCL_infoPrint("    EBCL_TASK_OPT_QM_JAIL = %s", (t->opts & EBCL_TASK_OPT_QM_JAIL) ? "true" : "false");
    EBCL_infoPrint("    EBCL_TASK_OPT_RESPAWN = %s", (t->opts & EBCL_TASK_OPT_RESPAWN) ? "true" : "false");
}

/**
 * Main function of crinit_parsecheck.
 *
 * Will try to parse and print out all task configurations given on the command line.
 */
int main(int argc, char *argv[], char *envp[]) {
    for (int n = 1; n < argc; n++) {
        ebcl_ConfKvList_t *c;
        if (EBCL_parseConf(&c, argv[n]) == -1) {
            EBCL_errPrint("Could not parse file \'%s\'.", argv[n]);
            return EXIT_FAILURE;
        }
        EBCL_infoPrint("File \'%s\' loaded successfully.", argv[n]);
        EBCL_infoPrint("---------");
        EBCL_infoPrint("Contents:");
        EBCL_infoPrint("---------");
        ebcl_ConfKvList_t *runner = c;
        do {
            if (runner->key != NULL && runner->val != NULL) {
                if (strncmp(runner->key, "COMMAND", 7) == 0) {
                    char **argArr = NULL;
                    int argCount = 0;
                    EBCL_infoPrint("\'%s\':", runner->key);
                    if (EBCL_confListExtractArgvArray(&argCount, &argArr, runner->key, c, true) == -1) {
                        EBCL_errPrint("Could not get argv-array for key \'%s\'.", runner->key);
                        EBCL_freeConfList(c);
                        return EXIT_FAILURE;
                    }
                    for (int i = 0; i < argCount; i++) {
                        EBCL_infoPrint("    ARGV[%d] = \'%s\'", i, argArr[i]);
                    }
                    EBCL_freeArgvArray(argArr);
                } else {
                    EBCL_infoPrint("\'%s\'=\'%s\'", runner->key, runner->val);
                }
            }
            runner = runner->next;
        } while (runner != NULL);
        EBCL_infoPrint("");
        EBCL_infoPrint("Will now attempt to extract a Task out of the config.");
        ebcl_Task_t *t = NULL;
        if (EBCL_taskCreateFromConfKvList(&t, c) == -1) {
            EBCL_errPrint("Could not extract task from ConfKvList.");
            EBCL_freeConfList(c);
            return EXIT_FAILURE;
        }
        EBCL_freeConfList(c);

        EBCL_infoPrint("Task extracted without error.");
        EBCL_taskPrint(t);

        EBCL_infoPrint("Will now attempt to duplicate the task and print out its (hopefully equal) contents.");
        ebcl_Task_t *u = NULL;
        if (EBCL_taskDup(&u, t) == -1) {
            EBCL_errPrint("Could not duplicate the task.");
            return EXIT_FAILURE;
        }
        EBCL_freeTask(t);
        EBCL_taskPrint(u);
        EBCL_freeTask(u);
    }
    EBCL_infoPrint("Done.");
    return EXIT_SUCCESS;
}
