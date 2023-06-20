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

/** Common NULL-pointer input check as all configuration handler functions have the same signature **/
#define EBCL_cfgHandlerCommonNullCheck() crinitNullCheck(-1, tgt, val)

/**
 * Helper function to set a bitmask value in crinitTask_t::opts.
 *
 * @param tgt  Direct pointer to the crinitTaskOpts_t inside an crinitTask_t which shall be modified.
 * @param opt  Bitmask of the task option to be set, one of `EBCL_TASK_OPT_*`
 * @param val  The string value indicating if the bit should be set or unset. Uses crinitConfConvToBool().
 *
 * @return  0 on success, -1 on error
 */
static inline int EBCL_cfgHandlerSetTaskOptFromStr(crinitTaskOpts_t *tgt, crinitTaskOpts_t opt, const char *val);
/**
 * (Re-)allocate memory for generic arrays.
 *
 * Given current size of an array and required size, the function will allocate/grow the array. Shrinking is unsupported
 * and will lead to an error being returned. Initializes newly allocated memory to 0.
 *
 * @param dynArr       The array to reallocate or NULL if we want a new array.
 * @param elementSize  Size of a single element in the array.
 * @param curSize      Current number of elements in the array.
 * @param reqSize      Required number of elements in the array.
 *
 * @return  The new address of the array on success, NULL on failure.
 */
static inline void *EBCL_cfgHandlerManageArrayMem(void *dynArr, size_t elementSize, size_t curSize, size_t reqSize);

int crinitTaskCfgCmdHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();

    size_t newIdx = tgt->cmdsSize;
    crinitTaskCmd_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->cmds, sizeof(*tgt->cmds), tgt->cmdsSize, newIdx + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      CRINIT_CONFIG_KEYSTR_COMMAND);
        return -1;
    }
    tgt->cmds = newArr;
    tgt->cmdsSize++;

    tgt->cmds[newIdx].argv = crinitConfConvToStrArr(&tgt->cmds[newIdx].argc, val, true);
    if (tgt->cmds[newIdx].argv == NULL) {
        crinitErrPrint("Could not extract argv/argc from '%s' index %zu.", CRINIT_CONFIG_KEYSTR_COMMAND, newIdx);
        return -1;
    }
    return 0;
}

int crinitTaskCfgDepHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();

    int tempDepsSize = 0;
    char **tempDeps = crinitConfConvToStrArr(&tempDepsSize, val, false);
    if (tempDeps == NULL) {
        crinitErrPrint("Could not extract string array from '%s' parameter, value: '%s'", CRINIT_CONFIG_KEYSTR_DEPENDS,
                      val);
        return -1;
    }

    // Skip if empty line
    if (tempDepsSize == 0) {
        crinitFreeArgvArray(tempDeps);
        return 0;
    }

    size_t oldSz = tgt->depsSize, newSz = (size_t)(oldSz + tempDepsSize);
    crinitTaskDep_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->deps, sizeof(*tgt->deps), oldSz, newSz);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      CRINIT_CONFIG_KEYSTR_DEPENDS);
        crinitFreeArgvArray(tempDeps);
        return -1;
    }
    tgt->deps = newArr;
    tgt->depsSize = newSz;

    for (size_t i = oldSz; i < tgt->depsSize; i++) {
        tgt->deps[i].name = strdup(tempDeps[i]);
        if (tgt->deps[i].name == NULL) {
            crinitErrnoPrint("Could not duplicate string for dependency '%s'.", tempDeps[i]);
            crinitFreeArgvArray(tempDeps);
            return -1;
        }

        char *strtokState = NULL;
        tgt->deps[i].name = strtok_r(tgt->deps[i].name, ":", &strtokState);
        tgt->deps[i].event = strtok_r(NULL, ":", &strtokState);

        if (tgt->deps[i].name == NULL || tgt->deps[i].event == NULL) {
            crinitErrPrint("Could not parse dependency '%s'.", tempDeps[i]);
            crinitFreeArgvArray(tempDeps);
            return -1;
        }
    }

    crinitFreeArgvArray(tempDeps);
    return 0;
}

int crinitTaskCfgPrvHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();

    int tempPrvsSize = 0;
    char **tempPrvs = crinitConfConvToStrArr(&tempPrvsSize, val, false);
    if (tempPrvs == NULL) {
        crinitErrPrint("Could not extract string array from '%s', value: '%s'.", CRINIT_CONFIG_KEYSTR_PROVIDES, val);
        return -1;
    }

    // Skip if empty line
    if (tempPrvsSize == 0) {
        return 0;
    }

    size_t oldSz = tgt->prvSize, newSz = (size_t)(oldSz + tempPrvsSize);
    crinitTaskPrv_t *newArr = EBCL_cfgHandlerManageArrayMem(tgt->prv, sizeof(*tgt->prv), oldSz, newSz);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      CRINIT_CONFIG_KEYSTR_PROVIDES);
        crinitFreeArgvArray(tempPrvs);
        return -1;
    }
    tgt->prv = newArr;
    tgt->prvSize = newSz;

    for (size_t i = oldSz; i < tgt->prvSize; i++) {
        crinitTaskPrv_t *ptr = &tgt->prv[i];
        ptr->stateReq = 0;
        ptr->name = strdup(tempPrvs[i]);
        if (ptr->name == NULL) {
            crinitErrnoPrint("Could not duplicate string for %s.", CRINIT_CONFIG_KEYSTR_PROVIDES);
            crinitFreeArgvArray(tempPrvs);
            return -1;
        }

        char *delimPtr = strchr(ptr->name, ':');
        if (delimPtr == NULL) {
            crinitErrnoPrint("Could not parse '%s' in %s.", ptr->name, CRINIT_CONFIG_KEYSTR_PROVIDES);
            crinitFreeArgvArray(tempPrvs);
            return -1;
        }
        *delimPtr++ = '\0';
        if (strncmp(delimPtr, CRINIT_TASK_EVENT_RUNNING, strlen(CRINIT_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = CRINIT_TASK_STATE_RUNNING;
        } else if (strncmp(delimPtr, CRINIT_TASK_EVENT_DONE, strlen(CRINIT_TASK_EVENT_RUNNING)) == 0) {
            ptr->stateReq = CRINIT_TASK_STATE_DONE;
        } else if (strncmp(delimPtr, CRINIT_TASK_EVENT_FAILED, strlen(CRINIT_TASK_EVENT_FAILED)) == 0) {
            ptr->stateReq = CRINIT_TASK_STATE_FAILED;
        } else {
            crinitErrnoPrint("Could not parse '%s' in %s.", ptr->name, CRINIT_CONFIG_KEYSTR_PROVIDES);
            crinitFreeArgvArray(tempPrvs);
            return -1;
        }

        delimPtr = strchr(delimPtr, '-');
        if (delimPtr != NULL && strcmp(delimPtr, CRINIT_TASK_EVENT_NOTIFY_SUFFIX) == 0) {
            ptr->stateReq |= CRINIT_TASK_STATE_NOTIFIED;
        }
    }

    crinitFreeArgvArray(tempPrvs);
    return 0;
}

int crinitTaskCfgEnvHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();
    if (tgt->taskEnv.envp == NULL &&
        crinitEnvSetInit(&tgt->taskEnv, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT) == -1) {
        crinitErrPrint("Could not initialize task environment.");
        return -1;
    }
    if (crinitConfConvToEnvSetMember(&tgt->taskEnv, val) == -1) {
        crinitErrPrint("Could not parse task environment directive '%s'.", val);
        return -1;
    }
    return 0;
}

int crinitTaskCfgIoRedirHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();

    size_t newIdx = tgt->redirsSize;
    crinitIoRedir_t *newArr =
        EBCL_cfgHandlerManageArrayMem(tgt->redirs, sizeof(*tgt->redirs), tgt->redirsSize, tgt->redirsSize + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                      CRINIT_CONFIG_KEYSTR_IOREDIR);
        return -1;
    }
    tgt->redirs = newArr;
    tgt->redirsSize++;

    if (crinitConfConvToIoRedir(&tgt->redirs[newIdx], val) == -1) {
        crinitErrPrint("Could not initialize IO redirection structure from '%s', value: '%s'.",
                      CRINIT_CONFIG_KEYSTR_IOREDIR, val);
        return -1;
    }
    return 0;
}

int crinitTaskCfgNameHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();
    tgt->name = strdup(val);
    if (tgt->name == NULL) {
        crinitErrnoPrint("Could not allocate memory for name of task '%s'.", val);
        return -1;
    }
    return 0;
}

int crinitTaskCfgRespHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();
    if (EBCL_cfgHandlerSetTaskOptFromStr(&tgt->opts, CRINIT_TASK_OPT_RESPAWN, val) == -1) {
        crinitErrPrint("Could not parse value of boolean option '%s'.", CRINIT_CONFIG_KEYSTR_RESPAWN);
        return -1;
    }
    return 0;
}

int crinitTaskCfgRespRetHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();
    if (crinitConfConvToInteger(&tgt->maxRetries, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.", CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES);
        return -1;
    }
    return 0;
}

int crinitTaskIncludeHandler(crinitTask_t *tgt, const char *val) {
    EBCL_cfgHandlerCommonNullCheck();

    int inclCfgSz;
    char **inclCfgStrArr = crinitConfConvToStrArr(&inclCfgSz, val, true);
    if (inclCfgStrArr == NULL) {
        crinitErrPrint("Could not extract config parameters from '%s', value: '%s'", CRINIT_CONFIG_KEYSTR_INCLUDE, val);
        return -1;
    }
    char *importList;
    if (inclCfgSz == 1) {  // INCLUDE without import list
        importList = NULL;
    } else if (inclCfgSz == 2) {  // INCLUDE with import list
        importList = inclCfgStrArr[1];
    } else {  // parser error
        crinitErrPrint("Unexpected number of parameters to '%s' config directive.", CRINIT_CONFIG_KEYSTR_INCLUDE);
        crinitFreeArgvArray(inclCfgStrArr);
        return -1;
    }
    if (crinitTaskMergeInclude(tgt, inclCfgStrArr[0], importList) == -1) {
        crinitErrPrint("Could not merge include '%s' into task.", inclCfgStrArr[0]);
        crinitFreeArgvArray(inclCfgStrArr);
        return -1;
    }
    crinitFreeArgvArray(inclCfgStrArr);
    return 0;
}

static inline int EBCL_cfgHandlerSetTaskOptFromStr(crinitTaskOpts_t *tgt, crinitTaskOpts_t opt, const char *val) {
    bool b;
    if (crinitConfConvToBool(&b, val) == -1) {
        crinitErrPrint("Could not convert from configuration file value to boolean.");
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
        crinitErrPrint("Configuration value arrays can only be grown in size.");
        return NULL;
    }

    void *out = NULL;
    if (curSize < reqSize) {
        out = realloc(dynArr, reqSize * elementSize);
        if (out == NULL) {
            crinitErrnoPrint(
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

