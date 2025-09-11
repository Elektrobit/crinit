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
#ifdef ENABLE_CAPABILITIES
#include <sys/capability.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef ENABLE_CAPABILITIES
#include "capabilities.h"
#endif
#ifdef ENABLE_CGROUP
#include "cgroup.h"
#endif
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
 * Modifies errno.
 *
 * @param uid UID to query
 * @param name Pointer to result. Caller must free the pointer allocated by the function.
 * @return true if UID could be resolved and converted, false otherwise.
 */
static bool crinitUidToUsername(uid_t uid, char **name);

/**
 * Check if given groupname exists and convert into numeric ID.
 *
 * Modifies errno.
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

#ifdef ENABLE_CAPABILITIES
int crinitCfgCapClearHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);

    crinitTask_t *t = tgt;
    crinitDbgInfoPrint("(Task %s) Configure capability clear '%s'.", t->name, val);

    if (crinitCapConvertToBitmask(&(t->capabilitiesClear), val) != 0) {
        return -1;
    }

    crinitDbgInfoPrint("(Task %s) Configured capabilities to be set: %#lx.", t->name, t->capabilitiesClear);
    return 0;
}

int crinitCfgCapSetHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);

    crinitTask_t *t = tgt;
    crinitDbgInfoPrint("(Task %s) Configure capability set '%s'.", t->name, val);

    if (crinitCapConvertToBitmask(&(t->capabilitiesSet), val) != 0) {
        return -1;
    }

    crinitDbgInfoPrint("(Task %s) Configured capabilities to be cleared: %#lx.", t->name, t->capabilitiesSet);
    return 0;
}

int crinitCfgDefaultCapsHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (crinitGlobOptSet(CRINIT_GLOBOPT_DEFAULTCAPS, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_DEFAULTCAPS);
        return -1;
    }
    crinitDbgInfoPrint("Configured default capabilities: %s.", val);
    return 0;
}
#endif

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
#ifndef ENABLE_ELOS
        if (strcmp(t->deps[i].name, "@elos") == 0) {
            crinitErrPrint("To depend on an ELOS filter ELOS support must be enabled at compile time.");
            crinitFreeArgvArray(tempDeps);
            return -1;
        }
#endif
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
#ifndef ENABLE_ELOS
    CRINIT_PARAM_UNUSED(tgt);
    CRINIT_PARAM_UNUSED(val);
    CRINIT_PARAM_UNUSED(type);
    crinitErrPrint("To support the option '%s' ELOS support must be activated at compile time.",
                   CRINIT_CONFIG_KEYSTR_FILTER_DEFINE);
    return -1;
#else
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
#endif
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

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert group list '%s' to string array.", val);
        return -1;
    }

    gid_t gid;

    if (confArrLen > 1) {
        t->supGroupsSize = confArrLen - 1;
        t->supGroups = calloc(t->supGroupsSize, sizeof(*t->supGroups));
        if (t->supGroups == NULL) {
            crinitErrPrint("Couldn not allocate memory for supplementary groups.\n");
            goto failInit;
        }
    }

    for (int i = 0; i < confArrLen; i++) {
        if (isalpha(*parsedVal[i])) {
            if (crinitGroupnameToGid(parsedVal[i], &gid)) {
                if (i == 0) {  // First run is main group
                    t->group = gid;
                    t->groupname = strdup(parsedVal[i]);
                    if (t->groupname == NULL) {
                        crinitErrPrint("Failed to allocate memory for groupname %s.", val);
                        goto failLoop;
                    }
                } else {
                    t->supGroups[i - 1] = gid;
                }
            }
        } else {
            // Make sure input is not a negative number
            long long temp = 0;
            if (crinitConfConvToInteger(&temp, parsedVal[i], 10) == -1) {
                crinitErrPrint("Invalid value for UID found");
                goto failLoop;
            } else if (temp < 0) {
                crinitErrPrint("Invalid (negative) value for UID found");
                goto failLoop;
            }

            if (crinitConfConvToInteger(&gid, parsedVal[i], 10) == -1) {
                crinitErrPrint("Invalid GID / group name found");
                goto failLoop;
            }
            if (i == 0) {  // First run is main group
                t->group = gid;
                if (crinitGidToGroupname(t->group, &t->groupname) != true) {
                    crinitErrPrint("Failed to map GID %d to a groupname.", t->group);
                    goto failLoop;
                }
            } else {
                char *tmp = NULL;
                if (crinitGidToGroupname(gid, &tmp) != true) {
                    crinitErrPrint("Failed to map GID %d to a groupname for supplementary group.", t->group);
                    free(tmp);
                    goto failLoop;
                }
                free(tmp);
                t->supGroups[i - 1] = gid;
            }
        }
    }

    // If GROUP is specified, we need at least one valid entry.
    if (t->groupname == NULL) {
        crinitErrPrint("Configuration key %s is empty.", CRINIT_CONFIG_KEYSTR_GROUP);
        goto failLoop;
    }

    crinitFreeArgvArray(parsedVal);
    return 0;

failLoop:
    free(t->supGroups);
    t->supGroups = NULL;
    t->supGroups = 0;
failInit:
    crinitFreeArgvArray(parsedVal);
    return -1;
}

#ifdef ENABLE_CGROUP
int crinitCfgCgroupNameHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;

    if (strlen(val) == 0) {
        crinitErrPrint("Failed to parse CGROUP_NAME: must not be empty");
        return -1;
    }

    if (t->cgroup == NULL) {
        t->cgroup = calloc(sizeof(*(t->cgroup)), 1);
        if (t->cgroup == NULL) {
            crinitErrPrint("Failed to allocate memory for task cgroup structure.");
            return -1;
        }
    }

    t->cgroup->name = strdup(val);
    if (t->cgroup->name == NULL) {
        crinitErrPrint("Failed to allocate memory for cgroup name %s", val);
        return -1;
    }

    return 0;
}

int crinitCfgCgroupParamsHandler(void *tgt, const char *val, crinitConfigType_t type) {
    crinitNullCheck(-1, tgt, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_TASK);
    crinitTask_t *t = tgt;

    // If parameters are specified, we need a cgroup name, too.
    if (t->cgroup == NULL || t->cgroup->name == NULL) {
        crinitErrPrint("Configuration key %s is empty.", CRINIT_CONFIG_KEYSTR_CGROUP_NAME);
        goto failInit;
    }

    crinitCgroupConfiguration_t *cgroupConfig = t->cgroup->config;

    if (cgroupConfig == NULL) {
        cgroupConfig = calloc(sizeof(*(cgroupConfig)), 1);
        if (cgroupConfig == NULL) {
            crinitErrPrint("Failed to allocate memory for cgroup configuration.");
            goto failInit;
        }
    }

    crinitCgroupParam_t **tmpParams = crinitCfgHandlerManageArrayMem(
        cgroupConfig->param, sizeof(*cgroupConfig->param), cgroupConfig->paramCount, cgroupConfig->paramCount + 1);
    if (tmpParams == NULL) {
        crinitErrPrint("Failed to (re)allocate memory for cgroup configuration parameters.");
        goto failInit;
    }
    cgroupConfig->param = tmpParams;
    cgroupConfig->paramCount++;
    cgroupConfig->param[cgroupConfig->paramCount - 1] = calloc(sizeof(*cgroupConfig->param), 1);

    if (crinitCgroupConvertSingleParamToObject(val, cgroupConfig->param[cgroupConfig->paramCount - 1]) != 0) {
        goto fail;
    }
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        crinitGlobOptRemit();
        goto fail;
    }
    t->cgroup->parent = globOpts->rootCgroup;
    t->cgroup->config = cgroupConfig;
    crinitGlobOptRemit();

    return 0;

fail:
    crinitFreeCgroupConfiguration(cgroupConfig);
    free(cgroupConfig);
failInit:
    return -1;
}

int crinitCfgCgroupRootNameHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    if (strlen(val) == 0) {
        crinitErrPrint("Failed to parse CGROUP_ROOT_NAME: must not be empty");
        return -1;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        return -1;
    }

    if (globOpts->rootCgroup == NULL) {
        globOpts->rootCgroup = calloc(sizeof(*(globOpts->rootCgroup)), 1);
        if (globOpts->rootCgroup == NULL) {
            crinitErrPrint("Failed to allocate memory for root cgroup configuration");
            crinitGlobOptRemit();
            return -1;
        }
    }

    crinitCgroup_t *rootConfig = globOpts->rootCgroup;

    rootConfig->name = strdup(val);
    if (rootConfig->name == NULL) {
        crinitErrPrint("Failed to allocate memory for root cgroup name %s", val);
        crinitGlobOptRemit();
        return -1;
    }

    crinitGlobOptRemit();
    return 0;
}

int crinitCfgCgroupRootParamsHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert group list '%s' to string array.", val);
        return -1;
    }

    if (confArrLen <= 0) {
        crinitErrPrint("Input values may not be empty");
        crinitFreeArgvArray(parsedVal);
        return -1;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        crinitFreeArgvArray(parsedVal);
        return -1;
    }

    if (globOpts->rootCgroup == NULL) {
        globOpts->rootCgroup = calloc(sizeof(*(globOpts->rootCgroup)), 1);
        if (globOpts->rootCgroup == NULL) {
            crinitErrPrint("Failed to allocate memory for root cgroup configuration");
            goto failInit;
        }
    }

    crinitCgroup_t *rootConfig = globOpts->rootCgroup;

    if (confArrLen > 0) {
        rootConfig->config = calloc(sizeof(*(rootConfig->config)), 1);
        if (rootConfig->config == NULL) {
            goto failInit;
        }
        if (crinitConvertConfigArrayToCGroupConfiguration(parsedVal, confArrLen, rootConfig->config) != 0) {
            goto fail;
        }
    }

    crinitFreeArgvArray(parsedVal);
    crinitGlobOptRemit();
    return 0;

fail:
    crinitFreeCgroupConfiguration(rootConfig->config);
    free(rootConfig->config);
    rootConfig->config = NULL;
failInit:
    crinitFreeArgvArray(parsedVal);
    crinitGlobOptRemit();
    return -1;
}

int crinitCfgCgroupGlobalNameHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert group list '%s' to string array.", val);
        return -1;
    }
    if (*parsedVal == NULL) {
        crinitErrPrint("Input value may not be empty.");
        crinitFreeArgvArray(parsedVal);
        return -1;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        crinitFreeArgvArray(parsedVal);
        return -1;
    }

    globOpts->globCgroupsCount = confArrLen;
    if (globOpts->globCgroupsCount == 0) {
        crinitErrPrint("Input values may not be empty");
        goto failInit;
    }
    if (globOpts->globCgroups == NULL) {
        globOpts->globCgroups = calloc(sizeof(*(globOpts->globCgroups)), confArrLen);
        if (globOpts->globCgroups == NULL) {
            crinitErrPrint("Failed to allocate memory for global cgroup configurations.");
            goto failInit;
        }
    }

    crinitCgroup_t **globConfigs = globOpts->globCgroups;

    for (int i = 0; i < confArrLen; i++) {
        globConfigs[i] = calloc(sizeof(**globConfigs), 1);
        if (globConfigs[i] == NULL) {
            crinitErrPrint("Failed to allocate memory for global cgroup configuration objects.");
            goto fail;
        }
        globConfigs[i]->name = strdup(parsedVal[i]);
        if (globConfigs[i]->name == NULL) {
            crinitErrPrint("Failed to copy global cgroup name '%s'.", parsedVal[i]);
            goto fail;
        }
    }

    crinitFreeArgvArray(parsedVal);
    crinitGlobOptRemit();
    return 0;

fail:
    for (int i = 0; i < confArrLen; i++) {
        crinitFreeCgroup(globConfigs[i]);
    }
    free(*globConfigs);
    free(globOpts->globCgroups);
    globOpts->globCgroups = NULL;
    globOpts->globCgroupsCount = 0;
failInit:
    crinitFreeArgvArray(parsedVal);
    crinitGlobOptRemit();
    return -1;
}

typedef struct {
    char *name;
    crinitCgroupParam_t **params;
    size_t numParams;
} crinitCgroupsGlobalParamArray_t;

static crinitCgroupsGlobalParamArray_t *crinitFindElementInCGroupsGlobalParamByName(
    char *name, crinitCgroupsGlobalParamArray_t **array, size_t arrayCount) {
    // While the input from the configuration file does not need to be ordered it is the most likely case that
    // entries for the same global cgroup configuration are in subsequent order. To optimize for this case
    // the entries of the temporary storage is checked from the back as that's the most likely position where
    // the element refered to by the param "name" can be found.
    // At least it should be more performant than to sort the array after each insertion with qsort() and then
    // search for the key "name" with bsearch.

    crinitNullCheck(NULL, name, array);

    for (size_t i = arrayCount; i > 0; i--) {
        if (array[i - 1]->name && strcmp(name, array[i - 1]->name) == 0) {
            return array[i - 1];
        }
    }

    return NULL;
}

/**
 * @brief Store all global cgroup parameters temporarily ordered by cgroup name
 *
 * The maximum number of elements in outCount is already known and the memory for out should be allocated
 * completely before calling this function.
 *
 * @param in Pointer to raw values from the configuration file
 * @param inCount Number of elements in parameter "in"
 * @param out Pointer to temporary array
 * @param outCount Number of elements in parameter "out"
 * @return On success 0, otherwise -1.
 */
static int crinitFillCgroupsGlobalParamTempArray(char **in, int inCount, crinitCgroupsGlobalParamArray_t **out,
                                                 size_t outCount) {
    crinitNullCheck(-1, in, out);

    for (int i = 0; i < inCount; i++) {
        char *delim = strchr(in[i], ':');
        if (delim == NULL) {
            crinitErrPrint("Invalid config entry for %s: %s", CRINIT_CONFIG_KEYSTR_CGROUP_GLOBAL_PARAMS, in[i]);
            return -1;
        }
        char *cgroupName = strndup(in[i], delim - in[i]);
        if (cgroupName == NULL) {
            crinitErrPrint("Failed to copy cgroup name into temporary memory.");
            return -1;
        }
        crinitCgroupsGlobalParamArray_t *elem = crinitFindElementInCGroupsGlobalParamByName(cgroupName, out, outCount);

        if (elem == NULL) {
            // overwrite pre-allocated blank element
            for (size_t j = 0; j < outCount; j++) {
                if (out[j]->name == NULL) {
                    elem = out[j];
                }
            }
            elem->name = cgroupName;

        } else {
            free(cgroupName);
            cgroupName = NULL;
        }
        // append config option
        if (elem->params == NULL) {
            elem->numParams = 1;
            elem->params = calloc(sizeof(*elem->params), elem->numParams);
        } else {
            elem->numParams++;
            elem->params = reallocarray(elem->params, elem->numParams, sizeof(*elem->params));
        }

        if (elem->params == NULL) {
            crinitErrPrint("Failed to allocate memory for cgroup params.");
            return -1;
        }

        elem->params[elem->numParams - 1] = calloc(sizeof(**elem->params), 1);
        if (elem->params[elem->numParams - 1] == NULL) {
            crinitErrPrint("Failed to allocate memory for temporary config element.");
            return -1;
        }

        if (crinitCgroupConvertSingleParamToObject(delim + 1, elem->params[elem->numParams - 1]) != 0) {
            crinitErrPrint("Failed to convert cgroup param to internal object.");
            return -1;
        }
    }

    return 0;
}

int crinitCfgCgroupGlobalParamsHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    int confArrLen;
    char **parsedVal = crinitConfConvToStrArr(&confArrLen, val, true);
    if (parsedVal == NULL) {
        crinitErrPrint("Could not convert group list '%s' to string array.", val);
        goto fail;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        goto fail;
    }

    // We expect to parsed the names before, therefore globCgroups may not be NULL
    if (globOpts->globCgroups == NULL) {
        crinitErrPrint("The names of the global cgroups need to be defined first.");
        goto failInit;
    }

    crinitCgroup_t **globConfigs = globOpts->globCgroups;

    crinitCgroupsGlobalParamArray_t **temp = NULL;
    temp = calloc(sizeof(*temp),
                  globOpts->globCgroupsCount);  // globCgroupsCount must hold the number of different global cgroups
    if (temp == NULL) {
        crinitErrPrint("Failed to allocate temporary memory.");
        goto failInit;
    }

    // create a temporary object for every known global cgroup (their names need to be defined first)
    for (size_t i = 0; i < globOpts->globCgroupsCount; i++) {
        temp[i] = calloc(sizeof(*temp[i]), 1);
        if (temp[i] == NULL) {
            crinitErrPrint("Failed to allocate temp memory for global cgroup options.");
            goto failLoop;
        }
        temp[i]->name = globOpts->globCgroups[i]->name;
    }

    if (crinitFillCgroupsGlobalParamTempArray(parsedVal, confArrLen, temp, globOpts->globCgroupsCount) != 0) {
        crinitErrPrint("Failed to store global cgroup parameters temporarily.");
        goto failLoop;
    }

    for (size_t i = 0; i < globOpts->globCgroupsCount; i++) {
        globConfigs[i]->config = calloc(sizeof(*globConfigs[i]->config), 1);
        globConfigs[i]->config->param = temp[i]->params;
        globConfigs[i]->config->paramCount = temp[i]->numParams;
        globConfigs[i]->parent = globOpts->rootCgroup;
        free(temp[i]);
    }
    free(temp);

    crinitFreeArgvArray(parsedVal);
    crinitGlobOptRemit();
    return 0;

failLoop:
    for (size_t i = 0; i < globOpts->globCgroupsCount; i++) {
        for (size_t j = 0; j < temp[i]->numParams; j++) {
            free(temp[i]->params[j]);
        }
        free(temp[i]->params);
        free(temp[i]);
    }
    free(temp);
    temp = NULL;

    for (int i = 0; i < confArrLen; i++) {
        crinitFreeCgroup(globConfigs[i]);
    }
    free(*globConfigs);
    free(globOpts->globCgroups);
    globOpts->globCgroups = NULL;
    globOpts->globCgroupsCount = 0;
failInit:
    crinitFreeArgvArray(parsedVal);
fail:
    crinitGlobOptRemit();
    return -1;
}
#endif

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
#ifndef ENABLE_ELOS
    CRINIT_PARAM_UNUSED(tgt);
    CRINIT_PARAM_UNUSED(val);
    CRINIT_PARAM_UNUSED(type);
    crinitErrPrint("To support the option '%s' ELOS support must be activated at compile time.",
                   CRINIT_CONFIG_KEYSTR_USE_ELOS);
    return -1;
#else
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
#endif
}

int crinitCfgElosServerHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

#ifndef ENABLE_ELOS
    crinitErrPrint("To support the option '%s' ELOS support must be activated at compile time.",
                   CRINIT_CONFIG_KEYSTR_ELOS_SERVER);
    return -1;
#else
    if (crinitGlobOptSet(CRINIT_GLOBOPT_ELOS_SERVER, val) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_SERVER);
        return -1;
    }
    return 0;
#endif
}

int crinitCfgElosPortHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

#ifndef ENABLE_ELOS
    crinitErrPrint("To support the option '%s' ELOS support must be activated at compile time.",
                   CRINIT_CONFIG_KEYSTR_ELOS_PORT);
    return -1;
#else
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
#endif
}

int crinitCfgElosEventPollIntervalHandler(void *tgt, const char *val, crinitConfigType_t type) {
    CRINIT_PARAM_UNUSED(tgt);
    crinitNullCheck(-1, val);
    crinitCfgHandlerTypeCheck(CRINIT_CONFIG_TYPE_SERIES);

    unsigned long long pollInterval;
    if (crinitConfConvToIntegerULL(&pollInterval, val, 10) == -1) {
        crinitErrPrint("Could not parse value of integral numeric option '%s'.",
                       CRINIT_CONFIG_KEYSTR_ELOS_EVENT_POLL_INTERVAL);
        return -1;
    }
    if (crinitGlobOptSet(CRINIT_GLOBOPT_ELOS_EVENT_POLL_INTERVAL, pollInterval) == -1) {
        crinitErrPrint("Could not set global option '%s'.", CRINIT_CONFIG_KEYSTR_ELOS_EVENT_POLL_INTERVAL);
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
