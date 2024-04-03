// SPDX-License-Identifier: MIT
/**
 * @file task.c
 * @brief Implementation of functions related to a single task.
 */
#include "task.h"

#include <stdlib.h>

#include "common.h"
#include "confmap.h"
#include "globopt.h"
#include "logio.h"

/**
 * Helper function to go through an crinitConfKvList_t and apply all contained settings to a target task.
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
static inline int crinitTaskSetFromConfKvList(crinitTask_t *tgt, const crinitConfKvList_t *src, crinitTaskType_t type,
                                              char *importList);

int crinitTaskCreateFromConfKvList(crinitTask_t **out, const crinitConfKvList_t *in) {
    crinitNullCheck(-1, out, in);

    *out = calloc(1, sizeof(**out));
    if (*out == NULL) {
        crinitErrnoPrint("Could not allocate memory for crinitTask.");
        return -1;
    }
    crinitTask_t *pTask = *out;
    pTask->pid = -1;
    pTask->maxRetries = -1;

    if (crinitGlobOptGet(CRINIT_GLOBOPT_ENV, &pTask->taskEnv) == -1) {
        crinitErrPrint("Could not retrieve global environment set during Task creation.");
        goto fail;
    }

    if (crinitGlobOptGet(CRINIT_GLOBOPT_FILTERS, &pTask->elosFilters) == -1) {
        crinitErrPrint("Could not retrieve global filters set during Task creation.");
        goto fail;
    }

    if (crinitTaskSetFromConfKvList(pTask, in, CRINIT_TASK_TYPE_STANDARD, NULL) == -1) {
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

    // Initialize timestamps and set creation time.
    if (clock_gettime(CLOCK_MONOTONIC, &pTask->createTime) == -1) {
        crinitErrnoPrint("Could not measure creation time of task '%s'. Will set to 0 (undefined) and continue.",
                         pTask->name);
        memset(&pTask->createTime, 0, sizeof(pTask->startTime));
    }
    memset(&pTask->startTime, 0, sizeof(pTask->startTime));
    memset(&pTask->endTime, 0, sizeof(pTask->startTime));

    return 0;

fail:
    crinitFreeTask(*out);
    *out = NULL;
    return -1;
}

int crinitTaskCopy(crinitTask_t *out, const crinitTask_t *orig) {
    memcpy(out, orig, sizeof(*out));
    out->name = NULL;
    out->deps = NULL;
    out->cmds = NULL;
    out->taskEnv.envp = NULL;
    out->elosFilters.envp = NULL;
    out->prv = NULL;
    out->redirs = NULL;

    out->name = strdup(orig->name);
    if (out->name == NULL) {
        crinitErrnoPrint("Could not allocate memory for task name during copy of Task \'%s\'.", orig->name);
        goto fail;
    }

    if (out->cmdsSize > 0) {
        out->cmds = calloc(out->cmdsSize, sizeof(*out->cmds));
        if (out->cmds == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu COMMANDs during copy of Task \'%s\'.", out->cmdsSize,
                             orig->name);
            goto fail;
        }

        for (size_t i = 0; i < out->cmdsSize; i++) {
            if (orig->cmds[i].argc < 1) {
                crinitErrPrint("COMMANDs must have at least one argument.");
                goto fail;
            }
            out->cmds[i].argc = orig->cmds[i].argc;
            out->cmds[i].argv = calloc((out->cmds[i].argc + 1), sizeof(*out->cmds[i].argv));
            if (out->cmds[i].argv == NULL) {
                crinitErrnoPrint(
                    "Could not allocate memory for argv-array of size %d during copy of task \'%s\', cmds[%zu].",
                    out->cmds[i].argc + 1, orig->name, i);
                goto fail;
            }

            char *origArgvBackbufEnd = strchr(orig->cmds[i].argv[out->cmds[i].argc - 1], '\0');
            size_t argvBackbufLen = origArgvBackbufEnd - orig->cmds[i].argv[0] + 1;

            char *argvBackbuf = malloc(argvBackbufLen);
            if (argvBackbuf == NULL) {
                crinitErrnoPrint("Could not allocate memory for cmds[%zu].argv of task \'%s\'.", i, orig->name);
                goto fail;
            }

            memcpy(argvBackbuf, orig->cmds[i].argv[0], argvBackbufLen);
            for (int j = 0; j < out->cmds[i].argc; j++) {
                out->cmds[i].argv[j] = argvBackbuf + (orig->cmds[i].argv[j] - orig->cmds[i].argv[0]);
            }
        }
    }

    if (crinitEnvSetDup(&out->taskEnv, &orig->taskEnv) == -1) {
        crinitErrPrint("Could not duplicate task environment during task duplication.");
        goto fail;
    }

    if (crinitEnvSetDup(&out->elosFilters, &orig->elosFilters) == -1) {
        crinitErrPrint("Could not duplicate elos filters during task duplication.");
        goto fail;
    }

    if (out->depsSize > 0) {
        out->deps = calloc(out->depsSize, sizeof(*out->deps));
        if (out->deps == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu TaskDeps during copy of Task \'%s\'.", out->depsSize,
                             orig->name);
            goto fail;
        }

        for (size_t i = 0; i < out->depsSize; i++) {
            size_t depNameLen = strlen(orig->deps[i].name) + 1;
            size_t depEventLen = strlen(orig->deps[i].event) + 1;
            out->deps[i].name = malloc(depNameLen + depEventLen);
            if (out->deps[i].name == NULL) {
                crinitErrnoPrint(
                    "Could not allocate memory for backing string in deps[%zu] during copy of Task \'%s\'.", i,
                    orig->name);
                goto fail;
            }
            memcpy(out->deps[i].name, orig->deps[i].name, depNameLen + depEventLen);
            out->deps[i].event = out->deps[i].name + depNameLen;
        }
    } else {
        out->deps = NULL;
    }

    if (out->prvSize > 0) {
        out->prv = calloc(out->prvSize, sizeof(*out->prv));
        if (out->prv == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu TaskPrvs during copy of Task \'%s\'.", out->prvSize,
                             orig->name);
            goto fail;
        }

        for (size_t i = 0; i < out->prvSize; i++) {
            out->prv[i].name = strdup(orig->prv[i].name);
            out->prv[i].stateReq = orig->prv[i].stateReq;
            if (out->prv[i].name == NULL) {
                crinitErrnoPrint("Could not allocate memory for TaskPrv at index %zu during copy of Task '%s'.", i,
                                 out->name);
                goto fail;
            }
        }
    } else {
        out->prv = NULL;
    }

    if (out->redirsSize > 0) {
        out->redirs = calloc(out->redirsSize, sizeof(*out->redirs));
        if (out->redirs == NULL) {
            crinitErrnoPrint("Could not allocate memory for %zu IO redirection(s) during copy of task '%s'.",
                             out->redirsSize, orig->name);
            goto fail;
        }
        for (size_t i = 0; i < out->redirsSize; i++) {
            if (crinitIoRedirCpy(&out->redirs[i], &orig->redirs[i]) == -1) {
                crinitErrPrint("Could not copy all IO redirections during copy of task '%s'.", orig->name);
                goto fail;
            }
        }
    }

    out->opts = orig->opts;
    out->state = orig->state;
    out->pid = orig->pid;
    out->maxRetries = orig->maxRetries;
    out->failCount = orig->failCount;

    return 0;

fail:
    crinitDestroyTask(out);
    return -1;
}

int crinitTaskDup(crinitTask_t **out, const crinitTask_t *orig) {
    *out = malloc(sizeof(crinitTask_t));
    if (*out == NULL) {
        crinitErrnoPrint("Could not allocate memory for duplicate of Task \'%s\'.", orig->name);
        return -1;
    }

    if (crinitTaskCopy(*out, orig) != 0) {
        crinitErrPrint("Failed to copy task \'%s\'.", orig->name);
        crinitFreeTask(*out);
        *out = NULL;
        return -1;
    }

    return 0;
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
            crinitFreeArgvArray(t->cmds[i].argv);
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
            crinitDestroyIoRedir(&(t->redirs[i]));
        }
    }
    free(t->redirs);
    crinitEnvSetDestroy(&t->taskEnv);
    crinitEnvSetDestroy(&t->elosFilters);
}

int crinitTaskMergeInclude(crinitTask_t *tgt, const char *src, char *importList) {
    crinitNullCheck(-1, tgt, src);

    char *inclDir = NULL, *inclSuffix = NULL, *inclPath = NULL;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_INCLDIR, &inclDir) == -1) {
        crinitErrPrint("Could not recall path include directory from global options.");
        return -1;
    }
    if (crinitGlobOptGet(CRINIT_GLOBOPT_INCL_SUFFIX, &inclSuffix) == -1) {
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

    crinitConfKvList_t *inclConfList;
    if (crinitParseConf(&inclConfList, inclPath) == -1) {
        crinitErrPrint("Could not parse include file at '%s'.", inclPath);
        free(inclPath);
        return -1;
    }

    if (crinitTaskSetFromConfKvList(tgt, inclConfList, CRINIT_TASK_TYPE_INCLUDE, importList) == -1) {
        crinitErrPrint("Could not merge include file '%s' into task.", inclPath);
        free(inclPath);
        crinitFreeConfList(inclConfList);
        return -1;
    }

    free(inclPath);
    crinitFreeConfList(inclConfList);
    return 0;
}

static inline int crinitTaskSetFromConfKvList(crinitTask_t *tgt, const crinitConfKvList_t *src, crinitTaskType_t type,
                                              char *importList) {
    crinitNullCheck(-1, tgt, src);

    bool importArr[CRINIT_CONFIGS_SIZE] = {false};
    bool duplCheckArr[CRINIT_CONFIGS_SIZE] = {false};
    if (type == CRINIT_TASK_TYPE_STANDARD || importList == NULL) {
        for (size_t i = 0; i < crinitNumElements(importArr); i++) {
            importArr[i] = true;
        }
    } else {
        char *strtokState;
        char *token = strtok_r(importList, ",", &strtokState);
        while (token != NULL) {
            const crinitConfigMapping_t *cfg = crinitFindConfigMapping(crinitTaskCfgMap, crinitTaskCfgMapSize, token);
            if (cfg == NULL) {
                crinitErrPrint("Unexpected configuration string in include import list: '%s'", token);
                return -1;
            }
            importArr[cfg->config] = true;
            token = strtok_r(NULL, ",", &strtokState);
        }
    }

    const crinitConfKvList_t *pEntry = src;
    const char *val = NULL;
    while (pEntry != NULL) {
        const crinitConfigMapping_t *tcm = crinitFindConfigMapping(crinitTaskCfgMap, crinitTaskCfgMapSize, pEntry->key);
        if (tcm == NULL) {
            crinitInfoPrint("Warning: Unknown configuration key '%s' encountered.", pEntry->key);
        } else {
            val = pEntry->val;
            if ((!tcm->includeSafe) && type == CRINIT_TASK_TYPE_INCLUDE) {
                crinitErrPrint("Non include-safe configuration parameter '%s' encountered in include file.",
                               pEntry->key);
                return -1;
            }
            if ((!tcm->arrayLike) && duplCheckArr[tcm->config]) {
                crinitErrPrint("Multiple values for non-array like configuration parameter '%s' given.", pEntry->key);
                return -1;
            }
            duplCheckArr[tcm->config] = true;
            if (importArr[tcm->config] && tcm->cfgHandler(tgt, val, CRINIT_CONFIG_TYPE_TASK) == -1) {
                crinitErrPrint("Could not parse configuration parameter '%s' with given value '%s'.", pEntry->key,
                               pEntry->val);
                return -1;
            }
        }
        pEntry = pEntry->next;
    }
    return 0;
}
