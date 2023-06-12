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
#include "globopt.h"
#include "lexers.h"
#include "logio.h"

#define crinitCfgHandlerTypeCheck(t)                                                                      \
    do {                                                                                                  \
        if ((t) != type) {                                                                                \
            crinitErrPrint("This function is not capable of dealing with the configuration type %s", #t); \
            return -1;                                                                                    \
        }                                                                                                 \
    } while (0)

/**
 * Helper function to set a bitmask value in crinitTask_t::opts.
 *
 * @param tgt  Direct pointer to the crinitTaskOpts_t inside an crinitTask_t which shall be modified.
 * @param opt  Bitmask of the task option to be set, one of `CRINIT_TASK_OPT_*`
 * @param val  The string value indicating if the bit should be set or unset. Uses crinitConfConvToBool().
 *
 * @return  0 on success, -1 on error
 */
static inline int crinitCfgHandlerSetTaskOptFromStr(crinitTaskOpts_t *tgt, crinitTaskOpts_t opt, const char *val);
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
static inline void *crinitCfgHandlerManageArrayMem(void *dynArr, size_t elementSize, size_t curSize, size_t reqSize);

int crinitCfgCmdHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
    size_t newIdx = t->cmdsSize;
    crinitTaskCmd_t *newArr = crinitCfgHandlerManageArrayMem(t->cmds, sizeof(*t->cmds), t->cmdsSize, newIdx + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_COMMAND);
        return -1;
    }
    t->cmds = newArr;
    t->cmdsSize++;

    t->cmds[newIdx].argv = crinitConfConvToStrArr(&t->cmds[newIdx].argc, val, true);
    if (t->cmds[newIdx].argv == NULL) {
        crinitErrPrint("Could not extract argv/argc from '%s' index %zu.", CRINIT_CONFIG_KEYSTR_COMMAND, newIdx);
        return -1;
    }
    return 0;
}

int crinitCfgDepHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;

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

    size_t oldSz = t->depsSize, newSz = (size_t)(oldSz + tempDepsSize);
    crinitTaskDep_t *newArr = crinitCfgHandlerManageArrayMem(t->deps, sizeof(*t->deps), oldSz, newSz);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_DEPENDS);
        crinitFreeArgvArray(tempDeps);
        return -1;
    }
    t->deps = newArr;
    t->depsSize = newSz;

    for (size_t i = oldSz; i < t->depsSize; i++) {
        t->deps[i].name = strdup(tempDeps[i - oldSz]);
        if (t->deps[i].name == NULL) {
            crinitErrnoPrint("Could not duplicate string for dependency '%s'.", tempDeps[i - oldSz]);
            crinitFreeArgvArray(tempDeps);
            return -1;
        }

        char *strtokState = NULL;
        t->deps[i].name = strtok_r(t->deps[i].name, ":", &strtokState);
        t->deps[i].event = strtok_r(NULL, ":", &strtokState);

        if (t->deps[i].name == NULL || t->deps[i].event == NULL) {
            crinitErrPrint("Could not parse dependency '%s'.", tempDeps[i - oldSz]);
            crinitFreeArgvArray(tempDeps);
            return -1;
        }
    }

    crinitFreeArgvArray(tempDeps);
    return 0;
}

int crinitCfgPrvHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);

    crinitTask_t *t = tgt;
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

    size_t oldSz = t->prvSize, newSz = (size_t)(oldSz + tempPrvsSize);
    crinitTaskPrv_t *newArr = crinitCfgHandlerManageArrayMem(t->prv, sizeof(*t->prv), oldSz, newSz);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_PROVIDES);
        crinitFreeArgvArray(tempPrvs);
        return -1;
    }
    t->prv = newArr;
    t->prvSize = newSz;

    for (size_t i = oldSz; i < t->prvSize; i++) {
        crinitTaskPrv_t *ptr = &t->prv[i];
        ptr->stateReq = 0;
        ptr->name = strdup(tempPrvs[i - oldSz]);
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

int crinitCfgEnvHandler(void *tgt, const char *val, crinitConfigType_t type) {
    if (type == CRINIT_CONFIG_TYPE_TASK) {
        crinitNullCheck(-1, tgt, val);
        crinitTask_t *t = tgt;
        if (crinitConfConvToEnvSetMember(&t->taskEnv, val) == -1) {
            crinitErrPrint("Could not parse task environment directive '%s'.", val);
            return -1;
        }
    } else if (type == CRINIT_CONFIG_TYPE_SERIES) {
        crinitNullCheck(-1, val);
        crinitEnvSet_t globEnv;
        if (crinitGlobOptGetEnvSet(&globEnv) == -1) {
            crinitErrPrint("Could not retrieve global task environment set.");
            return -1;
        }
        if (crinitConfConvToEnvSetMember(&globEnv, val) == -1) {
            crinitErrPrint("Could not parse task environment directive '%s'.", val);
            crinitEnvSetDestroy(&globEnv);
            return -1;
        }
        if (crinitGlobOptSetEnvSet(&globEnv) == -1) {
            crinitErrPrint("Could not store global task environment variables.");
            crinitEnvSetDestroy(&globEnv);
            return -1;
        }
        crinitEnvSetDestroy(&globEnv);
    } else {
        crinitErrPrint("Unexpected value for configuration file type.");
        return -1;
    }
    return 0;
}

int crinitCfgIoRedirHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);

    crinitTask_t *t = tgt;
    size_t newIdx = t->redirsSize;
    crinitIoRedir_t *newArr =
        crinitCfgHandlerManageArrayMem(t->redirs, sizeof(*t->redirs), t->redirsSize, t->redirsSize + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_IOREDIR);
        return -1;
    }
    t->redirs = newArr;
    t->redirsSize++;

    if (crinitConfConvToIoRedir(&t->redirs[newIdx], val) == -1) {
        crinitErrPrint("Could not initialize IO redirection structure from '%s', value: '%s'.",
                       CRINIT_CONFIG_KEYSTR_IOREDIR, val);
        return -1;
    }
    return 0;
}

int crinitCfgNameHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
    t->name = strdup(val);
    if (t->name == NULL) {
        crinitErrnoPrint("Could not allocate memory for name of task '%s'.", val);
        return -1;
    }
    return 0;
}

int crinitCfgRespHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
    if (crinitCfgHandlerSetTaskOptFromStr(&t->opts, CRINIT_TASK_OPT_RESPAWN, val) == -1) {
        crinitErrPrint("Could not parse value of boolean option '%s'.", CRINIT_CONFIG_KEYSTR_RESPAWN);
        return -1;
    }
    return 0;
}

int crinitCfgRespRetHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
    if (crinitConfConvToInteger(&t->maxRetries, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.", CRINIT_CONFIG_KEYSTR_RESPAWN_RETRIES);
        return -1;
    }
    return 0;
}

int crinitTaskIncludeHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
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
    if (crinitTaskMergeInclude(t, inclCfgStrArr[0], importList) == -1) {
        crinitErrPrint("Could not merge include '%s' into task.", inclCfgStrArr[0]);
        crinitFreeArgvArray(inclCfgStrArr);
        return -1;
    }
    crinitFreeArgvArray(inclCfgStrArr);
    return 0;
}

int crinitCfgDebugHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    bool v;
    if (crinitConfConvToBool(&v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }

    if (crinitGlobOptSetBoolean(CRINIT_GLOBOPT_DEBUG, &v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_DEBUG);
        return -1;
    }
    return 0;
}

int crinitCfgInclSuffixHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (val[0] != '.') {
        crinitErrPrint("Include file suffixes must begin with a dot ('.'). Offending value: '%s'", val);
        return -1;
    }
    if (crinitGlobOptSetString(CRINIT_GLOBOPT_INCL_SUFFIX, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_INCL_SUFFIX);
        return -1;
    }
    return 0;
}

int crinitCfgInclDirHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (!crinitIsAbsPath(val)) {
        crinitErrPrint("The value for '%s' must be an absolute path.", CRINIT_CONFIG_KEYSTR_INCLDIR);
        return -1;
    }
    if (crinitGlobOptSetString(CRINIT_GLOBOPT_INCLDIR, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_INCLDIR);
        return -1;
    }
    return 0;
}

int crinitCfgShdGpHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    unsigned long long gpMicros;
    if (crinitConfConvToInteger(&gpMicros, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.", CRINIT_CONFIG_KEYSTR_SHDGRACEP);
        return -1;
    }
    if (crinitGlobOptSetUnsignedLL(CRINIT_GLOBOPT_SHDGRACEP, &gpMicros) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_SHDGRACEP);
        return -1;
    }
    return 0;
}

int crinitCfgTaskSuffixHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (val[0] != '.') {
        crinitErrPrint("Task file suffixes must begin with a dot ('.'). Offending value: '%s'", val);
        return -1;
    }

    char **out = tgt;
    *out = strdup(val);
    return 0;
}

int crinitCfgTaskDirHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (!crinitIsAbsPath(val)) {
        crinitErrPrint("The value for '%s' must be an absolute path.", CRINIT_CONFIG_KEYSTR_TASKDIR);
        return -1;
    }

    char **out = tgt;
    *out = strdup(val);
    return 0;
}

int crinitCfgTaskDirSlHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    bool *v = tgt;
    if (crinitConfConvToBool(v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }
    return 0;
}

int crinitCfgTasksHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    char ***out = tgt;
    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert task list '%s' to string array.", val);
        return -1;
    }

    // If the tasks array is not yet initialized, just do the usual.
    if (*out == NULL) {
        *out = parsedVal;
        return 0;
    }

    // If we're here the Tasks array has already been initialized and we need to append.
    size_t oldSz = 0;
    while ((*out)[oldSz] != NULL) {
        oldSz++;
    }
    size_t newSz = oldSz + confArrLen;
    char **newArr = crinitCfgHandlerManageArrayMem(*out, sizeof(**out), oldSz + 1, newSz + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_TASKS);
        crinitFreeArgvArray(parsedVal);
        return -1;
    }

    size_t oldBackBufLen = strchr(newArr[oldSz - 1], '\0') - newArr[0] + 1;
    size_t addBackBufLen = strchr(parsedVal[confArrLen - 1], '\0') - parsedVal[0] + 1;
    size_t newBackBufLen = oldBackBufLen + addBackBufLen;
    char *newBackBuf = realloc(newArr[0], newBackBufLen);
    if (newBackBuf == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_TASKS);
        crinitFreeArgvArray(parsedVal);
        crinitFreeArgvArray(newArr);
        return -1;
    }
    memcpy(newBackBuf + oldBackBufLen, parsedVal[0], addBackBufLen);
    for (size_t i = 1; i < oldSz; i++) {
        newArr[i] = newBackBuf + (newArr[i] - newArr[0]);
    }
    for (size_t i = oldSz; i < newSz; i++) {
        newArr[i] = newBackBuf + oldBackBufLen + (parsedVal[i - oldSz] - parsedVal[0]);
    }
    newArr[0] = newBackBuf;

    crinitFreeArgvArray(parsedVal);
    *out = newArr;
    return 0;
}

int crinitCfgSyslogHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);
    bool v;
    if (crinitConfConvToBool(&v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }

    if (crinitGlobOptSetBoolean(CRINIT_GLOBOPT_USE_SYSLOG, &v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_USE_SYSLOG);
        return -1;
    }
    return 0;
}

static inline int crinitCfgHandlerSetTaskOptFromStr(crinitTaskOpts_t *tgt, crinitTaskOpts_t opt, const char *val) {
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

static inline void *crinitCfgHandlerManageArrayMem(void *dynArr, size_t elementSize, size_t curSize, size_t reqSize) {
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
