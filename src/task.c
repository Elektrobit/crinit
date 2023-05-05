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

#include "common.h"
#include "globopt.h"
#include "logio.h"
#include "confmap.h"

int EBCL_taskCreateFromConfKvList(ebcl_Task_t **out, const ebcl_ConfKvList_t *in) {
    EBCL_nullCheck(-1, out == NULL || in == NULL);

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
    pTask->prv = NULL;
    pTask->prvSize = 0;
    pTask->opts = 0;
    pTask->state = 0;
    pTask->pid = -1;
    pTask->redirs = NULL;
    pTask->redirsSize = 0;
    pTask->maxRetries = -1;
    pTask->failCount = 0;

    if (EBCL_globOptGetEnvSet(&pTask->taskEnv) == -1) {
        EBCL_errPrint("Could not retrieve global environment set during Task creation.");
        goto fail;
    }

    const ebcl_ConfKvList_t *pEntry = in;
    ebcl_CfgHdlCtx_t handlerCtx = {NULL, {0}, {0}};
    while (pEntry != NULL) {
        const ebcl_ConfigMapping_t *tcm = EBCL_findConfigMapping(pEntry->key);
        if (pEntry == NULL) {
            EBCL_infoPrint("Warning: Unknown configuration key '%s' encountered.", pEntry->key);
        } else {
            handlerCtx.val = pEntry->val;
            size_t idx = pEntry->keyArrIndex;
            if ((!tcm->arrayLike) && idx > 0) {
                EBCL_errPrint("Multiple values for non-array like configuration parameter '%s' given.", pEntry->key);
                goto fail;
            }
            handlerCtx.curIdx[tcm->config] = idx;
            if (handlerCtx.maxIdx[tcm->config] < idx) {
                handlerCtx.maxIdx[tcm->config] = idx;
            }
            if (tcm->cfgHandler(pTask, &handlerCtx) == -1) {
                EBCL_errPrint("Could not parse configuration parameter '%s' with given value '%s'.", pEntry->key,
                              pEntry->val);
                goto fail;
            }
        }
        pEntry = pEntry->next;
    }

    // Check that resulting task has a name and at least either a DEPENDS or COMMAND (as a meta-task only makes sense
    // with a DEPENDS and a regular task only with at least one COMMAND)
    if (pTask->name == NULL) {
        EBCL_errPrint("All task configurations must have a NAME.");
        goto fail;
    }
    if (pTask->cmdsSize == 0 && pTask->depsSize == 0) {
        EBCL_errPrint("The task '%s' seems to lack both COMMAND and DEPENDS which is unsupported.", pTask->name);
        goto fail;
    }

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
        pTask->prv = calloc(pTask->prvSize, sizeof(*pTask->prv));
        if (pTask->prv == NULL) {
            EBCL_errnoPrint("Could not allocate memory for %zu TaskPrvs during copy of Task \'%s\'.", pTask->prvSize,
                            orig->name);
            goto fail;
        }
    } else {
        pTask->prv = NULL;
    }

    for (size_t i = 0; i < pTask->prvSize; i++) {
        pTask->prv[i].name = strdup(orig->prv[i].name);
        pTask->prv[i].stateReq = orig->prv[i].stateReq;
        if (pTask->prv[i].name == NULL) {
            EBCL_errnoPrint("Could not allocate memory for TaskPrv at index %zu during copy of Task '%s'.", i,
                            pTask->name);
            goto fail;
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
        for (size_t i = 0; i < t->prvSize; i++) {
            free(t->prv[i].name);
        }
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

