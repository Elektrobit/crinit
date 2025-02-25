// SPDX-License-Identifier: MIT
/**
 * @file confhdl.c
 * @brief Implementation of type- and target-specific handler functions for configuration parsing.
 */
#include "confhdl.h"

#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "confconv.h"
#include "globopt.h"
#include "lexers.h"
#include "logio.h"

/**
 * Macro to assert that the given crinitConfigType_t in a crinitConfigHandler_t has a specific value.
 *
 * @param t  The target value of the parameter `type`. If unequal to the value given to the encompassing function, the
 *           function will print an error and return -1.
 */
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
/**
 * Check if a path exists, is accessible, and a directory.
 *
 * Uses stat() with S_ISDIR().
 *
 * @param path  The path to check.
 *
 * @return  true if \a path refers to an accessible directory, false otherwise.
 */
static bool crinitDirExists(const char *path);

/**
 * Check if a path exists, is accessible, a file and is executable by owner.
 *
 * Uses stat().
 *
 * @param path  The path to check.
 *
 * @return  true if \a path refers to an executable file, false otherwise.
 */
static bool crinitFileIsExecutable(const char *path);

/**
 * Check if given username exists and convert into numeric ID.
 *
 * @param name Username
 * @param uid Pointer to uid_t object
 *
 * @return true if username could be resolved, false otherwise.
 */
static bool crinitUsernameToUid(const char *name, uid_t *uid);

/**
 * Check if given UID exists and convert to username.
 *
 * @param uid UID to query
 * @param name Pointer to result. Caller must free the pointer allocated by the function.
 * @return true if UID could be resolved and converted, false otherwise.
 */
static bool crinitUidToUsername(uid_t uid, char **name);

/**
 * Check if given groupname exists and convert into numeric ID.
 *
 * @param name groupname
 * @param gid Pointer to gid_t object
 *
 * @return true if groupname could be resolved, false otherwise.
 */
static bool crinitGroupnameToGid(const char *name, gid_t *gid);

/**
 * Check if given GID exists and convert to groupname.
 *
 * @param gid GID to query
 * @param name Pointer to result. Caller must free the pointer allocated by the function.
 * @return true if GID could be resolved and converted, false otherwise.
 */
static bool crinitGidToGroupname(gid_t gid, char **name);

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

int crinitCfgStopCmdHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;
    size_t newIdx = t->stopCmdsSize;
    crinitTaskCmd_t *newArr =
        crinitCfgHandlerManageArrayMem(t->stopCmds, sizeof(*t->stopCmds), t->stopCmdsSize, newIdx + 1);
    if (newArr == NULL) {
        crinitErrPrint("Could not perform memory allocation during handler for configuration key '%s'.",
                       CRINIT_CONFIG_KEYSTR_STOP_COMMAND);
        return -1;
    }
    t->stopCmds = newArr;
    t->stopCmdsSize++;

    t->stopCmds[newIdx].argv = crinitConfConvToStrArr(&t->stopCmds[newIdx].argc, val, true);
    if (t->stopCmds[newIdx].argv == NULL) {
        crinitErrPrint("Could not extract argv/argc from '%s' index %zu.", CRINIT_CONFIG_KEYSTR_STOP_COMMAND, newIdx);
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
        crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
        if (globOpts == NULL) {
            crinitErrPrint("Could not get exclusive access to global option storage.");
            return -1;
        }
        if (crinitConfConvToEnvSetMember(&globOpts->globEnv, val) == -1) {
            crinitErrPrint("Could not parse task environment directive '%s'.", val);
            crinitGlobOptRemit();
            return -1;
        }
        if (crinitGlobOptRemit() == -1) {
            crinitErrPrint("Could not release exclusive access of global option storage.");
            return -1;
        }
    } else {
        crinitErrPrint("Unexpected value for configuration file type.");
        return -1;
    }
    return 0;
}

int crinitCfgFilterHandler(void *tgt, const char *val, crinitConfigType_t type) {
    if (type == CRINIT_CONFIG_TYPE_TASK) {
        crinitNullCheck(-1, tgt, val);
        crinitTask_t *t = tgt;
        if (crinitConfConvToEnvSetMember(&t->elosFilters, val) == -1) {
            crinitErrPrint("Could not parse task filters directive '%s'.", val);
            return -1;
        }
    } else if (type == CRINIT_CONFIG_TYPE_SERIES) {
        crinitNullCheck(-1, val);
        crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
        if (globOpts == NULL) {
            crinitErrPrint("Could not get exclusive access to global option storage.");
            return -1;
        }
        if (crinitConfConvToEnvSetMember(&globOpts->globFilters, val) == -1) {
            crinitErrPrint("Could not parse task filters directive '%s'.", val);
            crinitGlobOptRemit();
            return -1;
        }
        if (crinitGlobOptRemit() == -1) {
            crinitErrPrint("Could not release exclusive access of global option storage.");
            return -1;
        }
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

int crinitCfgUserHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;

    uid_t uid;
    if (isalpha(*val)) {
        if (crinitUsernameToUid(val, &uid)) {
            t->user = uid;
            t->username = strdup(val);
            if (t->username == NULL) {
                crinitErrPrint("Failed to allocate memory for username %s.", val);
                return -1;
            }
            return 0;
        }
    }

    // Make sure input is not a negative number
    long long temp = 0;
    if (crinitConfConvToInteger(&temp, val, 10) == -1) {
        crinitErrPrint("Invalid value for UID found");
        return -1;
    } else if (temp < 0) {
        crinitErrPrint("Invalid (negative) value for UID found");
        return -1;
    }

    if (crinitConfConvToInteger(&t->user, val, 10) == -1) {
        crinitErrPrint("Invalid UID / username found");
        return -1;
    }
    if (crinitUidToUsername(t->user, &t->username) != true) {
        crinitErrPrint("Failed to map UID %d to an username.", t->user);
        return -1;
    }
    return 0;
}

int crinitCfgGroupHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;

    gid_t gid;
    if (isalpha(*val)) {
        if (crinitGroupnameToGid(val, &gid)) {
            t->group = gid;
            t->groupname = strdup(val);
            if (t->groupname == NULL) {
                crinitErrPrint("Failed to allocate memory for groupname %s.", val);
                return -1;
            }
            return 0;
        }
    }

    // Make sure input is not a negative number
    long long temp = 0;
    if (crinitConfConvToInteger(&temp, val, 10) == -1) {
        crinitErrPrint("Invalid value for UID found");
        return -1;
    } else if (temp < 0) {
        crinitErrPrint("Invalid (negative) value for UID found");
        return -1;
    }

    if (crinitConfConvToInteger(&t->group, val, 10) == -1) {
        crinitErrPrint("Invalid GID / group name found");
        return -1;
    }
    if (crinitGidToGroupname(t->group, &t->groupname) != true) {
        crinitErrPrint("Failed to map GID %d to a groupname.", t->group);
        return -1;
    }
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

    if (crinitGlobOptSet(CRINIT_GLOBOPT_DEBUG, v) == -1) {
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
    if (crinitGlobOptSet(CRINIT_GLOBOPT_INCL_SUFFIX, val) == -1) {
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
    if (!crinitDirExists(val)) {
        crinitErrPrint("The value for '%s' is not a directory or inaccessible.", CRINIT_CONFIG_KEYSTR_INCLDIR);
        return -1;
    }
    if (crinitGlobOptSet(CRINIT_GLOBOPT_INCLDIR, val) == -1) {
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
    if (crinitGlobOptSet(CRINIT_GLOBOPT_SHDGRACEP, gpMicros) == -1) {
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

    if (crinitGlobOptSet(CRINIT_GLOBOPT_TASK_FILE_SUFFIX, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX);
        return -1;
    }

    return 0;
}

int crinitCfgTaskDirHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (!crinitIsAbsPath(val)) {
        crinitErrPrint("The value for '%s' must be an absolute path.", CRINIT_CONFIG_KEYSTR_TASKDIR);
        return -1;
    }
    if (!crinitDirExists(val)) {
        crinitErrPrint("The value for '%s' is not a directory or inaccessible.", CRINIT_CONFIG_KEYSTR_TASKDIR);
        return -1;
    }

    if (crinitGlobOptSet(CRINIT_GLOBOPT_TASKDIR, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_TASKDIR);
        return -1;
    }

    return 0;
}

int crinitCfgTaskDirSlHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    bool v;
    if (crinitConfConvToBool(&v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }

    if (crinitGlobOptSet(CRINIT_GLOBOPT_TASKDIR_FOLLOW_SYMLINKS, v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS);
        return -1;
    }

    return 0;
}

int crinitCfgTasksHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert task list '%s' to string array.", val);
        return -1;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        return -1;
    }

    char ***out = &globOpts->tasks;

    // If the tasks array is not yet initialized, just do the usual.
    if (*out == NULL) {
        *out = parsedVal;
        crinitGlobOptRemit();
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
        crinitGlobOptRemit();
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
        crinitGlobOptRemit();
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

    *out = newArr;

    if (crinitGlobOptRemit() == -1) {
        crinitErrPrint("Could not release exclusive access of global option storage.");
        return -1;
    }

    crinitFreeArgvArray(parsedVal);
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

    if (crinitGlobOptSet(CRINIT_GLOBOPT_USE_SYSLOG, v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_USE_SYSLOG);
        return -1;
    }
    return 0;
}

int crinitCfgElosHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);
    bool v;
    if (crinitConfConvToBool(&v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }

    if (crinitGlobOptSet(CRINIT_GLOBOPT_USE_ELOS, v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_USE_ELOS);
        return -1;
    }
    return 0;
}

int crinitCfgElosServerHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (crinitGlobOptSet(CRINIT_GLOBOPT_ELOS_SERVER, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_SERVER);
        return -1;
    }
    return 0;
}

int crinitCfgElosPortHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    int port;
    if (crinitConfConvToInteger(&port, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_PORT);
        return -1;
    }
    if (crinitGlobOptSet(CRINIT_GLOBOPT_ELOS_PORT, port) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_PORT);
        return -1;
    }
    return 0;
}

int crinitCfgElosPollIntervalHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    unsigned long long pollInterval;
    if (crinitConfConvToIntegerULL(&pollInterval, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_POLL_INTERVAL);
        return -1;
    }
    if (crinitGlobOptSet(CRINIT_GLOBOPT_ELOS_POLL_INTERVAL, pollInterval) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_POLL_INTERVAL);
        return -1;
    }
    return 0;
}

int crinitCfgLauncherCmdHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (!crinitIsAbsPath(val)) {
        crinitErrPrint("The value for '%s' must be an absolute path.", CRINIT_CONFIG_KEYSTR_LAUNCHER_CMD);
        return -1;
    }
    if (!crinitFileIsExecutable(val)) {
        crinitErrPrint("The value for '%s' is not a file or not exectuable.", CRINIT_CONFIG_KEYSTR_LAUNCHER_CMD);
        return -1;
    }
    if (crinitGlobOptSet(CRINIT_GLOBOPT_LAUNCHER_CMD, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_LAUNCHER_CMD);
        return -1;
    }
    return 0;
}

int crinitCfgSigKeyDirHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_KCMDLINE);

    if (crinitGlobOptSet(CRINIT_GLOBOPT_SIGKEYDIR, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_SIGKEYDIR);
        return -1;
    }
    return 0;
}

int crinitCfgSignaturesHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_KCMDLINE);

    bool v;
    if (crinitConfConvToBool(&v, val) == -1) {
        crinitErrPrint("Could not convert given string '%s' to a boolean value.", val);
        return -1;
    }

    if (crinitGlobOptSet(CRINIT_GLOBOPT_SIGNATURES, v) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_SIGNATURES);
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

    uint8_t *out = dynArr;
    if (curSize < reqSize) {
        out = realloc(dynArr, reqSize * elementSize);
        if (out == NULL) {
            crinitErrnoPrint(
                "Could not allocate additional memory to grow configuration value array from size %zu to size %zu.",
                curSize, reqSize);
            return NULL;
        }
        memset(&out[curSize * elementSize], 0, (reqSize - curSize) * elementSize);
        return out;
    }
    return out;
}

static bool crinitDirExists(const char *path) {
    crinitNullCheck(false, path);
    struct stat st;
    if (stat(path, &st) == -1) {
        crinitErrnoPrint("Could not stat '%s'.", path);
        return false;
    }
    if (!S_ISDIR(st.st_mode)) {
        crinitErrPrint("Given path '%s' is not a directory.", path);
        return false;
    }
    return true;
}

static bool crinitFileIsExecutable(const char *path) {
    crinitNullCheck(false, path);
    struct stat st;
    if (stat(path, &st) == -1) {
        crinitErrnoPrint("Could not stat '%s'.", path);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        crinitErrPrint("Given path '%s' is not a file.", path);
        return false;
    }

    if (st.st_mode & S_IXUSR) {
        return true;
    }

    return false;
}

static bool crinitUsernameToUid(const char *name, uid_t *uid) {
    crinitNullCheck(false, name, uid);
    struct passwd pwd;
    struct passwd *resPwd = NULL;
    char *buf;
    long bufsize;

    memset(&pwd, 0x00, sizeof(pwd));

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) { /* Value was indeterminate */
        bufsize = 16384; /* Should be more than enough */
    }

    buf = malloc(bufsize);
    if (buf == NULL) {
        crinitErrPrint("Failed to alloc memory for buffer.");
        return false;
    }

    bool result = false;

    const int rc = getpwnam_r(name, &pwd, buf, bufsize, &resPwd);
    if (resPwd == NULL) {
        switch (rc) {
            case 0:
            case ENOENT:
            case ESRCH:
            case EBADF:
            case EPERM:
                crinitErrPrint("Username %s couldn't be found.", name);
                result = false;
                goto cleanup;
                break;
            case EINTR:
            case EIO:
            case EMFILE:
            case ENFILE:
            case ENOMEM:
            case ERANGE:
                crinitErrPrint("System error while trying to resolve username.");
                result = false;
                goto cleanup;
                break;
            default:
                crinitErrPrint("Unknown failure %d while trying to resolve username.", rc);
                result = false;
                goto cleanup;
                break;
        }
    }
    *uid = resPwd->pw_uid;
    result = true;

cleanup:
    free(buf);
    return result;
}

static bool crinitUidToUsername(uid_t uid, char **name) {
    crinitNullCheck(false, name);
    struct passwd pwd;
    struct passwd *resPwd = NULL;
    char *buf;
    long bufsize;

    memset(&pwd, 0x00, sizeof(pwd));

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) { /* Value was indeterminate */
        bufsize = 16384; /* Should be more than enough */
    }

    buf = malloc(bufsize);
    if (buf == NULL) {
        crinitErrPrint("Failed to alloc memory for buffer.");
        return false;
    }

    bool result = false;

    const int rc = getpwuid_r(uid, &pwd, buf, bufsize, &resPwd);
    if (resPwd == NULL) {
        errno = rc;
        switch (rc) {
            case 0:
            case ENOENT:
            case ESRCH:
            case EBADF:
            case EPERM:
                crinitErrnoPrint("User ID %d couldn't be found.", uid);
                result = false;
                goto cleanup;
                break;
            case EINTR:
            case EIO:
            case EMFILE:
            case ENFILE:
            case ENOMEM:
            case ERANGE:
                crinitErrnoPrint("System error while trying to resolve username.");
                result = false;
                goto cleanup;
                break;
            default:
                crinitErrnoPrint("Unknown failure %d while trying to resolve username.", rc);
                result = false;
                goto cleanup;
                break;
        }
    }

    *name = strndup(resPwd->pw_name, bufsize);
    if (*name == NULL) {
        crinitErrPrint("Failed to alloc memory for username.");
        result = false;
        goto cleanup;
    }
    result = true;

cleanup:
    free(buf);
    return result;
}

static bool crinitGroupnameToGid(const char *name, gid_t *gid) {
    crinitNullCheck(false, name, gid);
    struct group grp;
    struct group *resGrp = NULL;
    char *buf;
    long bufsize;

    memset(&grp, 0x00, sizeof(grp));

    bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufsize == -1) { /* Value was indeterminate */
        bufsize = 16384; /* Should be more than enough */
    }

    buf = malloc(bufsize);
    if (buf == NULL) {
        crinitErrPrint("Failed to alloc memory for buffer.");
        return false;
    }

    bool result = false;

    const int rc = getgrnam_r(name, &grp, buf, bufsize, &resGrp);
    if (resGrp == NULL) {
        errno = rc;
        switch (rc) {
            case 0:
            case ENOENT:
            case ESRCH:
            case EBADF:
            case EPERM:
                crinitErrnoPrint("Groupname %s couldn't be found.", name);
                result = false;
                goto cleanup;
                break;
            case EINTR:
            case EIO:
            case EMFILE:
            case ENFILE:
            case ENOMEM:
            case ERANGE:
                crinitErrnoPrint("System error while trying to resolve goupname.");
                result = false;
                goto cleanup;
                break;
            default:
                crinitErrnoPrint("Unknown failure %d while trying to resolve groupname.", rc);
                result = false;
                goto cleanup;
                break;
        }
    }
    *gid = resGrp->gr_gid;
    result = true;

cleanup:
    free(buf);
    return result;
}

static bool crinitGidToGroupname(gid_t gid, char **name) {
    crinitNullCheck(false, name);
    struct group grp;
    struct group *resGrp = NULL;
    char *buf;
    long bufsize;

    memset(&grp, 0x00, sizeof(grp));

    bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufsize == -1) { /* Value was indeterminate */
        bufsize = 16384; /* Should be more than enough */
    }

    buf = malloc(bufsize);
    if (buf == NULL) {
        crinitErrPrint("Failed to alloc memory for buffer.");
        return false;
    }

    bool result = false;

    const int rc = getgrgid_r(gid, &grp, buf, bufsize, &resGrp);
    if (resGrp == NULL) {
        switch (rc) {
            case 0:
            case ENOENT:
            case ESRCH:
            case EBADF:
            case EPERM:
                crinitErrPrint("Group ID %d couldn't be found.", gid);
                result = false;
                goto cleanup;
                break;
            case EINTR:
            case EIO:
            case EMFILE:
            case ENFILE:
            case ENOMEM:
            case ERANGE:
                crinitErrPrint("System error while trying to resolve goupname.");
                result = false;
                goto cleanup;
                break;
            default:
                crinitErrPrint("Unknown failure %d while trying to resolve groupname.", rc);
                result = false;
                goto cleanup;
                break;
        }
    }
    *name = strndup(resGrp->gr_name, bufsize);
    if (*name == NULL) {
        crinitErrPrint("Failed to alloc memory for groupname.");
        result = false;
        goto cleanup;
    }
    result = true;

cleanup:
    free(buf);
    return result;
}
