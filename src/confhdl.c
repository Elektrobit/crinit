/**
 * @file confhdl.c
 * @brief Implementation of type- and target-specific handler functions for configuration parsing.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "confhdl.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confconv.h"
#include "lexers.h"
#include "logio.h"

#define EBCL_cfgHandlerCommonNullCheck() \
    EBCL_nullCheck(-1, tgt == NULL || handlerCtx == NULL || handlerCtx->val == NULL)

static inline int EBCL_cfgHandlerSetTaskOptFromStr(ebcl_TaskOpts_t *tgt, ebcl_TaskOpts_t opt, const char *val);
static inline void *EBCL_cfgHandlerManageArrayMem(void *dynArr, size_t elementSize, size_t curSize, size_t reqSize);

int EBCL_taskCfgCmdHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    if (handlerCtx->curIdx[EBCL_CONFIG_COMMAND] > handlerCtx->maxIdx[EBCL_CONFIG_COMMAND]) {
        EBCL_errPrint("Option index overflow internal to the task configuration parser.");
        return -1;
    }

    size_t newCmdsSize = handlerCtx->maxIdx[EBCL_CONFIG_COMMAND] + 1;
    ebcl_TaskCmd_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->cmds, sizeof(*tgt->cmds), tgt->cmdsSize, newCmdsSize);
    if (newArr == NULL) {
        EBCL_errPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      EBCL_CONFIG_KEYSTR_COMMAND);
        return -1;
    }
    tgt->cmds = newArr;
    tgt->cmdsSize = newCmdsSize;

    size_t newCmdIdx = handlerCtx->curIdx[EBCL_CONFIG_COMMAND];
    if (newCmdIdx >= tgt->cmdsSize) {
        EBCL_errPrint("No space left in array to acommodate element at index %zu. Array has size %zu.", newCmdIdx,
                      tgt->cmdsSize);
        return -1;
    }

    tgt->cmds[newCmdIdx].argv = EBCL_confConvToStrArr(&tgt->cmds[newCmdIdx].argc, handlerCtx->val, true);
    if (tgt->cmds[newCmdIdx].argv == NULL) {
        EBCL_errPrint("Could not extract argv/argc from '%s' index %zu.", EBCL_CONFIG_KEYSTR_COMMAND, newCmdIdx);
        return -1;
    }
    return 0;
}

int EBCL_taskCfgDepHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();

    int tempDepsSize = 0;
    char **tempDeps = EBCL_confConvToStrArr(&tempDepsSize, handlerCtx->val, false);
    if (tempDeps == NULL) {
        EBCL_errPrint("Could not extract string array from '%s' index %zu.", EBCL_CONFIG_KEYSTR_DEPENDS,
                      handlerCtx->curIdx[EBCL_CONFIG_DEPENDS]);
        return -1;
    }

    // Skip if empty line
    if (tempDepsSize == 0) {
        EBCL_freeArgvArray(tempDeps);
        return 0;
    }

    size_t oldSz = tgt->depsSize, newSz = (size_t)(oldSz + tempDepsSize);
    ebcl_TaskDep_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->deps, sizeof(*tgt->deps), oldSz, newSz);
    if (newArr == NULL) {
        EBCL_errPrint("Could not perform memory allocation during handler for configuration key '%s', index %zu.",
                      EBCL_CONFIG_KEYSTR_DEPENDS, handlerCtx->curIdx[EBCL_CONFIG_DEPENDS]);
        EBCL_freeArgvArray(tempDeps);
        return -1;
    }
    tgt->deps = newArr;
    tgt->depsSize = newSz;

    for (size_t i = oldSz; i < tgt->depsSize; i++) {
        tgt->deps[i].name = strdup(tempDeps[i]);
        if (tgt->deps[i].name == NULL) {
            EBCL_errnoPrint("Could not duplicate string for dependency '%s'.", tempDeps[i]);
            EBCL_freeArgvArray(tempDeps);
            return -1;
        }

        char *strtokState = NULL;
        tgt->deps[i].name = strtok_r(tgt->deps[i].name, ":", &strtokState);
        tgt->deps[i].event = strtok_r(NULL, ":", &strtokState);

        if (tgt->deps[i].name == NULL || tgt->deps[i].event == NULL) {
            EBCL_errPrint("Could not parse dependency '%s'.", tempDeps[i]);
            EBCL_freeArgvArray(tempDeps);
            return -1;
        }
    }

    EBCL_freeArgvArray(tempDeps);
    return 0;
}

int EBCL_taskCfgPrvHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();

    int tempPrvsSize = 0;
    char **tempPrvs = EBCL_confConvToStrArr(&tempPrvsSize, handlerCtx->val, false);
    if (tempPrvs == NULL) {
        EBCL_errPrint("Could not extract string array from '%s' index %zu.", EBCL_CONFIG_KEYSTR_PROVIDES,
                      handlerCtx->curIdx[EBCL_CONFIG_PROVIDES]);
        return -1;
    }

    // Skip if empty line
    if (tempPrvsSize == 0) {
        return 0;
    }

    size_t oldSz = tgt->prvSize, newSz = (size_t)(oldSz + tempPrvsSize);
    ebcl_TaskPrv_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->prv, sizeof(*tgt->prv), oldSz, newSz);
    if (newArr == NULL) {
        EBCL_errPrint("Could not perform memory allocation during handler for configuration key '%s', index %zu.",
                      EBCL_CONFIG_KEYSTR_PROVIDES, handlerCtx->curIdx[EBCL_CONFIG_PROVIDES]);
        EBCL_freeArgvArray(tempPrvs);
        return -1;
    }
    tgt->prv = newArr;
    tgt->prvSize = newSz;

    for (size_t i = oldSz; i < tgt->prvSize; i++) {
        ebcl_TaskPrv_t *ptr = &tgt->prv[i];
        ptr->stateReq = 0;
        ptr->name = strdup(tempPrvs[i]);
        if (ptr->name == NULL) {
            EBCL_errnoPrint("Could not duplicate string for %s.", EBCL_CONFIG_KEYSTR_PROVIDES);
            EBCL_freeArgvArray(tempPrvs);
            return -1;
        }

        char *delimPtr = strchr(ptr->name, ':');
        if (delimPtr == NULL) {
            EBCL_errnoPrint("Could not parse '%s' in %s.", ptr->name, EBCL_CONFIG_KEYSTR_PROVIDES);
            EBCL_freeArgvArray(tempPrvs);
            return -1;
        }
        *delimPtr++ = '\0';
        if (strncmp(delimPtr, EBCL_TASK_EVENT_RUNNING, strlen(EBCL_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_RUNNING;
        } else if (strncmp(delimPtr, EBCL_TASK_EVENT_DONE, strlen(EBCL_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_DONE;
        } else if (strncmp(delimPtr, EBCL_TASK_EVENT_FAILED, strlen(EBCL_TASK_EVENT_FAILED)) == 0) {
            ptr->stateReq = EBCL_TASK_STATE_FAILED;
        } else {
            EBCL_errnoPrint("Could not parse '%s' in %s.", ptr->name, EBCL_CONFIG_KEYSTR_PROVIDES);
            EBCL_freeArgvArray(tempPrvs);
            return -1;
        }

        delimPtr = strchr(delimPtr, '-');
        if (delimPtr != NULL && strcmp(delimPtr, EBCL_TASK_EVENT_NOTIFY_SUFFIX) == 0) {
            ptr->stateReq |= EBCL_TASK_STATE_NOTIFIED;
        }
    }

    EBCL_freeArgvArray(tempPrvs);
    return 0;
}

int EBCL_taskCfgEnvHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    if (tgt->taskEnv.envp == NULL &&
        EBCL_envSetInit(&tgt->taskEnv, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT) == -1) {
        EBCL_errPrint("Could not initialize task environment.");
        return -1;
    }
    if (EBCL_confConvToEnvSetMember(&tgt->taskEnv, handlerCtx->val) == -1) {
        EBCL_errPrint("Could not parse task environment directive '%s'.", handlerCtx->val);
        return -1;
    }
    return 0;
}

int EBCL_taskCfgIoRedirHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    if (handlerCtx->curIdx[EBCL_CONFIG_IOREDIR] > handlerCtx->maxIdx[EBCL_CONFIG_IOREDIR]) {
        EBCL_errPrint("Option index overflow internal to the task configuration parser.");
        return -1;
    }

    size_t newRedirsSize = handlerCtx->maxIdx[EBCL_CONFIG_IOREDIR] + 1;
    ebcl_IoRedir_t *newArr =
        EBCL_cfgHandlerManageArrayMem(tgt->redirs, sizeof(*tgt->redirs), tgt->redirsSize, newRedirsSize);
    if (newArr == NULL) {
        EBCL_errPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      EBCL_CONFIG_KEYSTR_IOREDIR);
        return -1;
    }
    tgt->redirs = newArr;
    tgt->redirsSize = newRedirsSize;

    size_t newRedirIdx = handlerCtx->curIdx[EBCL_CONFIG_IOREDIR];
    if (newRedirIdx >= tgt->redirsSize) {
        EBCL_errPrint("No space left in array to acommodate element at index %zu. Array has size %zu.", newRedirIdx,
                      tgt->redirsSize);
        return -1;
    }

    if (EBCL_confConvToIoRedir(&tgt->redirs[newRedirIdx], handlerCtx->val) == -1) {
        EBCL_errPrint("Could not initialize IO redirection structure from '%s' index %zu.", EBCL_CONFIG_KEYSTR_IOREDIR,
                      newRedirIdx);
        return -1;
    }
    return 0;
}

int EBCL_taskCfgNameHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    tgt->name = strdup(handlerCtx->val);
    if (tgt->name == NULL) {
        EBCL_errnoPrint("Could not allocate memory for name of task '%s'.", handlerCtx->val);
        return -1;
    }
    return 0;
}

int EBCL_taskCfgRespHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    if (EBCL_cfgHandlerSetTaskOptFromStr(&tgt->opts, EBCL_TASK_OPT_RESPAWN, handlerCtx->val) == -1) {
        EBCL_errPrint("Could not parse value of boolean option '%s'.", EBCL_CONFIG_KEYSTR_RESPAWN);
        return -1;
    }
    return 0;
}

int EBCL_taskCfgRespRetHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    if (EBCL_confConvToInteger(&tgt->maxRetries, handlerCtx->val, 10) == -1) {
        EBCL_errPrint("Could not parse value of integral numeric option '%s'.", EBCL_CONFIG_KEYSTR_RESPAWN_RETRIES);
        return -1;
    }
    return 0;
}

int EBCL_taskIncludeHandler(ebcl_Task_t *tgt, ebcl_CfgHdlCtx_t *handlerCtx) {
    EBCL_cfgHandlerCommonNullCheck();
    return 0;
}

static inline int EBCL_cfgHandlerSetTaskOptFromStr(ebcl_TaskOpts_t *tgt, ebcl_TaskOpts_t opt, const char *val) {
    bool b;
    if (EBCL_confConvToBool(&b, val) == -1) {
        EBCL_errPrint("Could not convert from configuration file value to boolean.");
        return -1;
    }
    if (b) {
        *tgt |= opt;
    } else {
        *tgt &= ~opt;
    }
    return 0;
}

static inline void *EBCL_cfgHandlerManageArrayMem(void *dynArr, size_t elementSize, size_t curSize, size_t reqSize) {
    if (reqSize < curSize) {
        EBCL_errPrint("Configuration value arrays can only be grown in size.");
        return NULL;
    }
    void *out = NULL;
    if (dynArr == NULL) {
        out = calloc(reqSize, elementSize);
        if (out == NULL) {
            EBCL_errnoPrint("Could not allocate memory for a configuration value array with %zu elements.", reqSize);
        }
        return out;
    }

    if (curSize < reqSize) {
        out = realloc(dynArr, reqSize * elementSize);
        if (out == NULL) {
            EBCL_errnoPrint(
                "Could not allocate additional memory to grow configuration value array from size %zu to size %zu.",
                curSize, reqSize);
            return NULL;
        }
        uint8_t *tempPtr = out;
        memset(&tempPtr[curSize * elementSize], 0, (reqSize - curSize) * elementSize);
        return out;
    }
    return out;
}
