/**
 * @file task.c
 * @brief Implementation of functions related to a single task.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "task.h"

#include <stdlib.h>

#include "logio.h"
#include "globopt.h"

int EBCL_taskCreateFromConfKvList(ebcl_Task_t **out, const ebcl_ConfKvList_t *in) {
    if (in == NULL) {
        EBCL_errPrint("The parameter \'in\' must not be NULL.");
        return -1;
    }

    *out = malloc(sizeof(ebcl_Task_t));
    if (*out == NULL) {
        EBCL_errnoPrint("Could not allocate memory for ebcl_Task.");
        return -1;
    }
    ebcl_Task_t *pTask = *out;
    pTask->name = NULL;
    pTask->deps = NULL;
    pTask->depsSize = 0;
    pTask->cmds = NULL;
    pTask->cmdsSize = 0;
    pTask->taskEnv.envp = NULL;
    pTask->taskEnv.allocSz = 0;
    pTask->taskEnv.allocInc = 0;
    pTask->prv = NULL;
    pTask->prvSize = 0;
    pTask->opts = 0;
    pTask->state = 0;
    pTask->pid = -1;
    pTask->redirs = NULL;
    pTask->redirsSize = 0;
    pTask->maxRetries = -1;
    pTask->failCount = 0;

    char *tempName = NULL;
    if (EBCL_confListGetVal(&tempName, "NAME", in) == -1) {
        EBCL_errPrint("Mandatory key \'NAME\' not found in task config file.");
        goto fail;
    }
    pTask->name = strdup(tempName);
    if (pTask->name == NULL) {
        EBCL_errnoPrint("Could not allocate memory for name of task \'%s\'.", tempName);
        goto fail;
    }

    bool tempYesNo = false;
    if (EBCL_confListExtractBoolean(&tempYesNo, "RESPAWN", true, in) == -1) {
        EBCL_errPrint("Could not extract mandatory boolean key \'RESPAWN\' from task config file.");
        goto fail;
    }
    pTask->opts |= (tempYesNo) ? EBCL_TASK_OPT_RESPAWN : 0;

    if (EBCL_confListExtractSignedInt(&pTask->maxRetries, 10, "RESPAWN_RETRIES", false, in) == -1) {
        EBCL_errPrint("Failed to parse integer value for non-mandatory key \'RESPAWN_RETRIES\' from task config file.");
        goto fail;
    }

    ssize_t cmdArrSize = EBCL_confListKeyGetMaxIdx(in, "COMMAND");
    if (cmdArrSize >= 0) {
        pTask->cmdsSize = (size_t)(cmdArrSize + 1);
        pTask->cmds = calloc(pTask->cmdsSize, sizeof(*pTask->cmds));
        if (pTask->cmds == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu commands in task %s.", pTask->cmdsSize, tempName);
            goto fail;
        }

        for (size_t i = 0; i < pTask->cmdsSize; i++) {
            if (EBCL_confListExtractArgvArrayWithIdx(&(pTask->cmds[i].argc), &(pTask->cmds[i].argv), "COMMAND", i, true,
                                                     in, true) == -1) {
                EBCL_errPrint(
                    "Could not extract argv/argc from COMMAND[%zu] in config for task "
                    "\'%s\'.",
                    i, tempName);
                goto fail;
            }
        }

        ssize_t redirsMaxIdx = EBCL_confListKeyGetMaxIdx(in, EBCL_CONFIG_KEYSTR_IOREDIR);
        if (redirsMaxIdx > -1) {
            pTask->redirsSize = (size_t)(redirsMaxIdx + 1);
            pTask->redirs = calloc(pTask->redirsSize, sizeof(*pTask->redirs));
            if (pTask->redirs == NULL) {
                EBCL_errnoPrint("Could not allocate memory for IO redirections for task '%s'.", tempName);
                goto fail;
            }
            for (size_t i = 0; i < pTask->redirsSize; i++) {
                if (EBCL_initIoRedirFromConfKvList(&(pTask->redirs[i]), EBCL_CONFIG_KEYSTR_IOREDIR, i, in) == -1) {
                    EBCL_errPrint("Could not initialize IO redirection definition %zu from config for task '%s'.", i,
                                  tempName);
                    goto fail;
                }
            }
        }
    }

    char **tempDeps = NULL;
    int tempDepsSize = 0;
    if (EBCL_confListExtractArgvArray(&tempDepsSize, &tempDeps, "DEPENDS", true, in, false) == -1) {
        EBCL_errPrint("Could not extract DEPENDS string from config file for task \'%s\'.", tempName);
        goto fail;
    }
    pTask->depsSize = (size_t)tempDepsSize;
    if (pTask->depsSize > 0) {
        pTask->deps = malloc(pTask->depsSize * sizeof(ebcl_TaskDep_t));
        if (pTask->deps == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu TaskDeps during copy of Task \'%s\'.", pTask->depsSize,
                            pTask->name);
            goto fail;
        }
    } else {
        pTask->deps = NULL;
    }

    for (size_t i = 0; i < pTask->depsSize; i++) {
        pTask->deps[i].name = NULL;
        pTask->deps[i].event = NULL;
    }

    for (size_t i = 0; i < pTask->depsSize; i++) {
        size_t depLen = strlen(tempDeps[i]) + 1;
        pTask->deps[i].name = malloc(depLen);

        if (pTask->deps[i].name == NULL) {
            EBCL_errnoPrint("Could not allocate memory for dependency \'%s\' in task \'%s\'.", tempDeps[i],
                            pTask->name);
        }
        memcpy(pTask->deps[i].name, tempDeps[i], depLen);

        char *strtokState = NULL;
        pTask->deps[i].name = strtok_r(pTask->deps[i].name, ":", &strtokState);
        pTask->deps[i].event = strtok_r(NULL, ":", &strtokState);

        if (pTask->deps[i].name == NULL || pTask->deps[i].event == NULL) {
            EBCL_errPrint("Could not parse dependency \'%s\' in task \'%s\'.", tempDeps[i], pTask->name);
            EBCL_freeArgvArray(tempDeps);
            goto fail;
        }
    }

    EBCL_freeArgvArray(tempDeps);

    ebcl_EnvSet_t globEnv;
    if (EBCL_globOptGetEnvSet(&globEnv) == -1) {
        EBCL_errPrint("Could not retrieve global environment set for task \'%s\'.", pTask->name);
        goto fail;
    }
    if (EBCL_envSetCreateFromConfKvList(&pTask->taskEnv, &globEnv, in) == -1) {
        EBCL_errPrint("Could not parse local task environment for task \'%s\'.", pTask->name);
        EBCL_envSetDestroy(&globEnv);
        goto fail;
    }
    EBCL_envSetDestroy(&globEnv);

    char **prvArgv = NULL;
    int prvSize = 0;

    // PROVIDES is non-mandatory
    if (EBCL_confListExtractArgvArray(&prvSize, &prvArgv, "PROVIDES", false, in, false) == -1) {
        EBCL_errPrint("Could not extract PROVIDES string from config file for task \'%s\'.", tempName);
        goto fail;
    }
    if (prvSize == 0 || prvArgv == NULL) {
        return 0;
    }

    pTask->prvSize = (size_t)prvSize;
    pTask->prv = malloc(pTask->prvSize * sizeof(ebcl_TaskPrv_t));
    if (pTask->prv == NULL) {
        EBCL_errnoPrint("Could not allocate memory for array of provided features in task \'%s\'.", pTask->name);
        goto fail;
    }

    for (size_t i = 0; i < pTask->prvSize; i++) {
        ebcl_TaskPrv_t *ptr = &pTask->prv[i];
        ptr->stateReq = 0;
        ptr->name = prvArgv[i];
        char *delimPtr = strchr(ptr->name, ':');
        if (delimPtr == NULL) {
            EBCL_errnoPrint("Could not parse \'%s\' in PROVIDES of task \'%s\'.", ptr->name, pTask->name);
            free(prvArgv);
            goto fail;
        }
        *delimPtr++ = '\0';
        if (strncmp(delimPtr, EBCL_TASK_EVENT_RUNNING, strlen(EBCL_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_RUNNING;
        } else if (strncmp(delimPtr, EBCL_TASK_EVENT_DONE, strlen(EBCL_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_DONE;
        } else if (strncmp(delimPtr, EBCL_TASK_EVENT_FAILED, strlen(EBCL_TASK_EVENT_FAILED)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_FAILED;
        } else {
            EBCL_errnoPrint("Could not parse \'%s\' in PROVIDES of task \'%s\'.", ptr->name, pTask->name);
            free(prvArgv);
            goto fail;
        }

        delimPtr = strchr(delimPtr, '-');
        if (delimPtr != NULL && strcmp(delimPtr, EBCL_TASK_EVENT_NOTIFY_SUFFIX) == 0) {
            ptr->stateReq |= EBCL_TASK_STATE_NOTIFIED;
        }
    }
    free(prvArgv);

    return 0;
fail:
    EBCL_freeTask(*out);
    *out = NULL;
    return -1;
}

int EBCL_taskDup(ebcl_Task_t **out, const ebcl_Task_t *orig) {
    *out = malloc(sizeof(ebcl_Task_t));
    if (*out == NULL) {
        EBCL_errnoPrint("Could not allocate memory for duplicate of Task \'%s\'.", orig->name);
        return -1;
    }
    ebcl_Task_t *pTask = *out;
    pTask->name = NULL;
    pTask->deps = NULL;
    pTask->depsSize = 0;
    pTask->cmds = NULL;
    pTask->cmdsSize = 0;
    pTask->taskEnv.envp = NULL;
    pTask->taskEnv.allocSz = 0;
    pTask->taskEnv.allocInc = 0;
    pTask->prv = NULL;
    pTask->prvSize = 0;
    pTask->opts = 0;
    pTask->state = 0;
    pTask->pid = -1;
    pTask->redirs = NULL;
    pTask->redirsSize = 0;
    pTask->maxRetries = -1;
    pTask->failCount = 0;

    size_t nameLen = strlen(orig->name) + 1;
    pTask->name = malloc(nameLen);
    if (pTask->name == NULL) {
        EBCL_errnoPrint("Could not allocate memory for task name during copy of Task \'%s\'.", orig->name);
        goto fail;
    }
    memcpy(pTask->name, orig->name, nameLen);

    pTask->cmdsSize = orig->cmdsSize;
    if (pTask->cmdsSize > 0) {
        pTask->cmds = calloc(pTask->cmdsSize, sizeof(*pTask->cmds));
        if (pTask->cmds == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu COMMANDs during copy of Task \'%s\'.", pTask->cmdsSize,
                            orig->name);
            goto fail;
        }

        for (size_t i = 0; i < pTask->cmdsSize; i++) {
            if (orig->cmds[i].argc < 1) {
                EBCL_errPrint("COMMANDs must have at least one argument.");
                goto fail;
            }
            pTask->cmds[i].argc = orig->cmds[i].argc;
            pTask->cmds[i].argv = calloc((pTask->cmds[i].argc + 1), sizeof(*pTask->cmds[i].argv));
            if (pTask->cmds[i].argv == NULL) {
                EBCL_errnoPrint(
                    "Could not allocate memory for argv-array of size %d during copy of task \'%s\', cmds[%zu].",
                    pTask->cmds[i].argc + 1, orig->name, i);
                goto fail;
            }

            char *origArgvBackbufEnd = strchr(orig->cmds[i].argv[pTask->cmds[i].argc - 1], '\0');
            size_t argvBackbufLen = origArgvBackbufEnd - orig->cmds[i].argv[0] + 1;

            char *argvBackbuf = malloc(argvBackbufLen);
            if (argvBackbuf == NULL) {
                EBCL_errnoPrint("Could not allocate memory for cmds[%zu].argv of task \'%s\'.", i, orig->name);
                goto fail;
            }

            memcpy(argvBackbuf, orig->cmds[i].argv[0], argvBackbufLen);
            char *runner = argvBackbuf;
            for (int j = 0; j < pTask->cmds[i].argc; j++) {
                size_t argvLen = strlen(orig->cmds[i].argv[j]) + 1;
                pTask->cmds[i].argv[j] = runner;
                runner += argvLen;
            }
        }
    }

    if (EBCL_envSetDup(&pTask->taskEnv, &orig->taskEnv) == -1) {
        EBCL_errPrint("Could not duplicate task environment during task duplication.");
        goto fail;
    }

    pTask->depsSize = orig->depsSize;
    if (pTask->depsSize > 0) {
        pTask->deps = malloc(pTask->depsSize * sizeof(ebcl_TaskDep_t));
        if (pTask->deps == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu TaskDeps during copy of Task \'%s\'.", pTask->depsSize,
                            orig->name);
            goto fail;
        }
    } else {
        pTask->deps = NULL;
    }

    for (size_t i = 0; i < pTask->depsSize; i++) {
        pTask->deps[i].name = NULL;
        pTask->deps[i].event = NULL;
    }

    for (size_t i = 0; i < pTask->depsSize; i++) {
        size_t depNameLen = strlen(orig->deps[i].name) + 1;
        size_t depEventLen = strlen(orig->deps[i].event) + 1;
        pTask->deps[i].name = malloc(depNameLen + depEventLen);
        if (pTask->deps[i].name == NULL) {
            EBCL_errnoPrint("Could not allocate memory for backing string in deps[%zu] during copy of Task \'%s\'.", i,
                            orig->name);
            goto fail;
        }
        memcpy(pTask->deps[i].name, orig->deps[i].name, depNameLen);
        memcpy(pTask->deps[i].name + depNameLen, orig->deps[i].event, depEventLen);
        pTask->deps[i].event = pTask->deps[i].name + depNameLen;
    }

    pTask->prvSize = orig->prvSize;
    if (pTask->prvSize > 0) {
        pTask->prv = malloc(pTask->prvSize * sizeof(ebcl_TaskPrv_t));
        if (pTask->prv == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu TaskPrvs during copy of Task \'%s\'.", pTask->prvSize,
                            orig->name);
            goto fail;
        }
    } else {
        pTask->prv = NULL;
    }

    for (size_t i = 0; i < pTask->prvSize; i++) {
        pTask->prv[i].name = NULL;
        pTask->prv[i].stateReq = 0;
    }

    size_t backStrSize = 0;
    for (size_t i = 0; i < orig->prvSize; i++) {
        backStrSize += strlen(orig->prv[i].name) + 1;
    }

    if (backStrSize > 0) {
        pTask->prv[0].name = malloc(backStrSize);
        if (pTask->prv[0].name == NULL) {
            EBCL_errnoPrint("Could not allocate memory for backing string in prv[0] during copy of Task \'%s\'.",
                            orig->name);
            goto fail;
        }
        char *pos = pTask->prv[0].name;
        for (size_t i = 0; i < pTask->prvSize; i++) {
            pTask->prv[i].name = pos;
            pos = stpcpy(pTask->prv[i].name, orig->prv[i].name) + 1;
            pTask->prv[i].stateReq = orig->prv[i].stateReq;
        }
    }

    pTask->redirsSize = orig->redirsSize;
    if (pTask->redirsSize > 0) {
        pTask->redirs = calloc(pTask->redirsSize, sizeof(*pTask->redirs));
        if (pTask->redirs == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu IO redirection(s) during copy of task '%s'.",
                            pTask->redirsSize, orig->name);
            goto fail;
        }
        for (size_t i = 0; i < pTask->redirsSize; i++) {
            if (EBCL_ioRedirCpy(&pTask->redirs[i], &orig->redirs[i]) == -1) {
                EBCL_errPrint("Could not copy all IO redirections during copy of task '%s'.", orig->name);
                goto fail;
            }
        }
    }

    pTask->opts = orig->opts;
    pTask->state = orig->state;
    pTask->pid = orig->pid;
    pTask->maxRetries = orig->maxRetries;
    pTask->failCount = orig->failCount;

    return 0;

fail:
    EBCL_freeTask(*out);
    *out = NULL;
    return -1;
}

void EBCL_freeTask(ebcl_Task_t *t) {
    if (t == NULL) {
        return;
    }
    EBCL_destroyTask(t);
    free(t);
}

void EBCL_destroyTask(ebcl_Task_t *t) {
    if (t == NULL) return;
    free(t->name);
    if (t->cmds != NULL) {
        for (size_t i = 0; i < t->cmdsSize; i++) {
            EBCL_freeArgvArray(t->cmds[i].argv);
        }
    }
    free(t->cmds);
    if (t->deps != NULL) {
        for (size_t i = 0; i < t->depsSize; i++) {
            free(t->deps[i].name);
        }
    }
    free(t->deps);
    if (t->prv != NULL) {
        free(t->prv[0].name);  // free backing string
    }
    free(t->prv);
    if (t->redirs != NULL) {
        for (size_t i = 0; i < t->redirsSize; i++) {
            EBCL_destroyIoRedir(&(t->redirs[i]));
        }
    }
    free(t->redirs);
    EBCL_envSetDestroy(&t->taskEnv);
}

