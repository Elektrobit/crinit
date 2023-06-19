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
#include "confmap.h"
#include "globopt.h"
#include "logio.h"

/**
 * Helper function to go through an ebcl_ConfKvList_t and apply all contained settings to a target task.
 *
 * Will call appropriate config handlers (see confhdl.h).
 *
 * @param tgt         The target task to be modified.
 * @param src         The list of config parameters from a task config file.
 * @param type        The crinitTaskType_t of the source task configuration file, i.e. if it is a regular task or an
 *                    include file. Relevant for checking include safety of options and for \a importList behavior.
 * @param importList  A comma-separated list of option names to be used from src. If NULL, all are used. Only relevant
 *                    if `type == CRINIT_TASK_TYPE_INCLUDE`.
 *
 * @return  0 on success, -1 on error
 */
static inline int EBCL_taskSetFromConfKvList(crinitTask_t *tgt, const ebcl_ConfKvList_t *src, crinitTaskType_t type,
                                             char *importList);

int crinitTaskCreateFromConfKvList(crinitTask_t **out, const ebcl_ConfKvList_t *in) {
    crinitNullCheck(-1, out, in);

    *out = calloc(1, sizeof(**out));
    if (*out == NULL) {
        crinitErrnoPrint("Could not allocate memory for ebcl_Task.");
        return -1;
    }
    crinitTask_t *pTask = *out;
    pTask->pid = -1;
    pTask->maxRetries = -1;

    if (EBCL_globOptGetEnvSet(&pTask->taskEnv) == -1) {
        crinitErrPrint("Could not retrieve global environment set during Task creation.");
        goto fail;
    }

    if (EBCL_taskSetFromConfKvList(pTask, in, CRINIT_TASK_TYPE_STANDARD, NULL) == -1) {
        crinitErrPrint("Could not set parameters of new task from configuration list.");
        goto fail;
    }

    // Check that resulting task has a name and at least either a DEPENDS or COMMAND (as a meta-task only makes sense
    // with a DEPENDS and a regular task only with at least one COMMAND)
    if (pTask->name == NULL) {
        crinitErrPrint("All task configurations must have a NAME.");
        goto fail;
    }
    if (pTask->cmdsSize == 0 && pTask->depsSize == 0) {
        crinitErrPrint("The task '%s' seems to lack both COMMAND and DEPENDS which is unsupported.", pTask->name);
        goto fail;
    }

    return 0;
fail:
    crinitFreeTask(*out);
    *out = NULL;
    return -1;
}

int crinitTaskDup(crinitTask_t **out, const crinitTask_t *orig) {
    *out = malloc(sizeof(crinitTask_t));
    if (*out == NULL) {
        crinitErrnoPrint("Could not allocate memory for duplicate of Task \'%s\'.", orig->name);
        return -1;
    }
    crinitTask_t *pTask = *out;
    memcpy(pTask, orig, sizeof(*pTask));
    pTask->name = NULL;
    pTask->deps = NULL;
    pTask->cmds = NULL;
    pTask->taskEnv.envp = NULL;
    pTask->prv = NULL;
    pTask->redirs = NULL;

    pTask->name = strdup(orig->name);
    if (pTask->name == NULL) {
        crinitErrnoPrint("Could not allocate memory for task name during copy of Task \'%s\'.", orig->name);
        goto fail;
    }

    if (pTask->cmdsSize > 0) {
        pTask->cmds = calloc(pTask->cmdsSize, sizeof(*pTask->cmds));
        if (pTask->cmds == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu COMMANDs during copy of Task \'%s\'.", pTask->cmdsSize,
                            orig->name);
            goto fail;
        }

        for (size_t i = 0; i < pTask->cmdsSize; i++) {
            if (orig->cmds[i].argc < 1) {
                crinitErrPrint("COMMANDs must have at least one argument.");
                goto fail;
            }
            pTask->cmds[i].argc = orig->cmds[i].argc;
            pTask->cmds[i].argv = calloc((pTask->cmds[i].argc + 1), sizeof(*pTask->cmds[i].argv));
            if (pTask->cmds[i].argv == NULL) {
                crinitErrnoPrint(
                    "Could not allocate memory for argv-array of size %d during copy of task \'%s\', cmds[%zu].",
                    pTask->cmds[i].argc + 1, orig->name, i);
                goto fail;
            }

            char *origArgvBackbufEnd = strchr(orig->cmds[i].argv[pTask->cmds[i].argc - 1], '\0');
            size_t argvBackbufLen = origArgvBackbufEnd - orig->cmds[i].argv[0] + 1;

            char *argvBackbuf = malloc(argvBackbufLen);
            if (argvBackbuf == NULL) {
                crinitErrnoPrint("Could not allocate memory for cmds[%zu].argv of task \'%s\'.", i, orig->name);
                goto fail;
            }

            memcpy(argvBackbuf, orig->cmds[i].argv[0], argvBackbufLen);
            for (int j = 0; j < pTask->cmds[i].argc; j++) {
                pTask->cmds[i].argv[j] = argvBackbuf + (orig->cmds[i].argv[j] - orig->cmds[i].argv[0]);
            }
        }
    }

    if (crinitEnvSetDup(&pTask->taskEnv, &orig->taskEnv) == -1) {
        crinitErrPrint("Could not duplicate task environment during task duplication.");
        goto fail;
    }

    if (pTask->depsSize > 0) {
        pTask->deps = calloc(pTask->depsSize, sizeof(*pTask->deps));
        if (pTask->deps == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu TaskDeps during copy of Task \'%s\'.", pTask->depsSize,
                            orig->name);
            goto fail;
        }

        for (size_t i = 0; i < pTask->depsSize; i++) {
            size_t depNameLen = strlen(orig->deps[i].name) + 1;
            size_t depEventLen = strlen(orig->deps[i].event) + 1;
            pTask->deps[i].name = malloc(depNameLen + depEventLen);
            if (pTask->deps[i].name == NULL) {
                crinitErrnoPrint("Could not allocate memory for backing string in deps[%zu] during copy of Task \'%s\'.",
                                i, orig->name);
                goto fail;
            }
            memcpy(pTask->deps[i].name, orig->deps[i].name, depNameLen + depEventLen);
            pTask->deps[i].event = pTask->deps[i].name + depNameLen;
        }
    } else {
        pTask->deps = NULL;
    }

    if (pTask->prvSize > 0) {
        pTask->prv = calloc(pTask->prvSize, sizeof(*pTask->prv));
        if (pTask->prv == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu TaskPrvs during copy of Task \'%s\'.", pTask->prvSize,
                            orig->name);
            goto fail;
        }

        for (size_t i = 0; i < pTask->prvSize; i++) {
            pTask->prv[i].name = strdup(orig->prv[i].name);
            pTask->prv[i].stateReq = orig->prv[i].stateReq;
            if (pTask->prv[i].name == NULL) {
                crinitErrnoPrint("Could not allocate memory for TaskPrv at index %zu during copy of Task '%s'.", i,
                                pTask->name);
                goto fail;
            }
        }
    } else {
        pTask->prv = NULL;
    }

    if (pTask->redirsSize > 0) {
        pTask->redirs = calloc(pTask->redirsSize, sizeof(*pTask->redirs));
        if (pTask->redirs == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu IO redirection(s) during copy of task '%s'.",
                            pTask->redirsSize, orig->name);
            goto fail;
        }
        for (size_t i = 0; i < pTask->redirsSize; i++) {
            if (EBCL_ioRedirCpy(&pTask->redirs[i], &orig->redirs[i]) == -1) {
                crinitErrPrint("Could not copy all IO redirections during copy of task '%s'.", orig->name);
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
    crinitFreeTask(*out);
    *out = NULL;
    return -1;
}

void crinitFreeTask(crinitTask_t *t) {
    if (t == NULL) {
        return;
    }
    crinitDestroyTask(t);
    free(t);
}

void crinitDestroyTask(crinitTask_t *t) {
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
    crinitEnvSetDestroy(&t->taskEnv);
}

int crinitTaskMergeInclude(crinitTask_t *tgt, const char *src, char *importList) {
    crinitNullCheck(-1, tgt, src);

    char *inclDir = NULL, *inclSuffix = NULL, *inclPath = NULL;
    if (EBCL_globOptGetString(EBCL_GLOBOPT_INCLDIR, &inclDir) == -1) {
        crinitErrPrint("Could not recall path include directory from global options.");
        return -1;
    }
    if (EBCL_globOptGetString(EBCL_GLOBOPT_INCL_SUFFIX, &inclSuffix) == -1) {
        crinitErrPrint("Could not recall include file suffix from global options.");
        free(inclDir);
        return -1;
    }

    size_t pathLen = snprintf(NULL, 0, "%s/%s%s", inclDir, src, inclSuffix);
    inclPath = malloc(pathLen + 1);
    if (inclPath == NULL) {
        crinitErrnoPrint("Could not allocate memory for full include file path.");
        free(inclDir);
        free(inclSuffix);
        return -1;
    }

    sprintf(inclPath, "%s/%s%s", inclDir, src, inclSuffix);

    free(inclDir);
    free(inclSuffix);

    ebcl_ConfKvList_t *inclConfList;
    if (EBCL_parseConf(&inclConfList, inclPath) == -1) {
        crinitErrPrint("Could not parse include file at '%s'.", inclPath);
        free(inclPath);
        return -1;
    }

    if (EBCL_taskSetFromConfKvList(tgt, inclConfList, CRINIT_TASK_TYPE_INCLUDE, importList) == -1) {
        crinitErrPrint("Could not merge include file '%s' into task.", inclPath);
        free(inclPath);
        EBCL_freeConfList(inclConfList);
        return -1;
    }

    free(inclPath);
    EBCL_freeConfList(inclConfList);
    return 0;
}

static inline int EBCL_taskSetFromConfKvList(crinitTask_t *tgt, const ebcl_ConfKvList_t *src, crinitTaskType_t type,
                                             char *importList) {
    crinitNullCheck(-1, tgt, src);

    bool importArr[EBCL_CONFIGS_SIZE] = {false};
    if (type == CRINIT_TASK_TYPE_STANDARD || importList == NULL) {
        for (size_t i = 0; i < crinitNumElements(importArr); i++) {
            importArr[i] = true;
        }
    } else {
        char *strtokState;
        char *token = strtok_r(importList, ",", &strtokState);
        while (token != NULL) {
            const ebcl_ConfigMapping_t *cfg = EBCL_findConfigMapping(token);
            if (cfg == NULL) {
                crinitErrPrint("Unexpected configuration string in include import list: '%s'", token);
                return -1;
            }
            importArr[cfg->config] = true;
            token = strtok_r(NULL, ",", &strtokState);
        }
    }

    const ebcl_ConfKvList_t *pEntry = src;
    const char *val = NULL;
    while (pEntry != NULL) {
        const ebcl_ConfigMapping_t *tcm = EBCL_findConfigMapping(pEntry->key);
        if (tcm == NULL) {
            crinitInfoPrint("Warning: Unknown configuration key '%s' encountered.", pEntry->key);
        } else {
            val = pEntry->val;
            if ((!tcm->includeSafe) && type == CRINIT_TASK_TYPE_INCLUDE) {
                crinitErrPrint("Non include-safe configuration parameter '%s' encountered in include file.",
                              pEntry->key);
                return -1;
            }
            if ((!tcm->arrayLike) && pEntry->keyArrIndex > 0) {
                crinitErrPrint("Multiple values for non-array like configuration parameter '%s' given.", pEntry->key);
                return -1;
            }
            if (importArr[tcm->config] && tcm->cfgHandler(tgt, val) == -1) {
                crinitErrPrint("Could not parse configuration parameter '%s' with given value '%s'.", pEntry->key,
                              pEntry->val);
                return -1;
            }
        }
        pEntry = pEntry->next;
    }
    return 0;
}

