// SPDX-License-Identifier: MIT
/**
 * @file confparse.c
 * @brief Implementation of the Config Parser.
 */
#include "confparse.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confconv.h"
#include "confmap.h"
#include "elosdep.h"
#include "envset.h"
#include "globopt.h"
#include "ini.h"
#include "ioredir.h"
#include "lexers.h"
#include "logio.h"

/**
 * Struct definition for the parser context used by crinitIniHandler()
 */
typedef struct {
    crinitConfKvList_t *anchor;  ///< Anchor pointer to the beginning of the list.
    crinitConfKvList_t *pList;   ///< Running pointer to the element being currently constructed.
    crinitConfKvList_t *last;    ///< Running pointer to the last element just constructed.
    size_t keyArrayCount;        ///< Counter variable for array-like config options.
} crinitIniParserCtx_t;

/**
 * Parser handler for libinih.
 */
static int crinitIniHandler(void *parserCtx, const char *section, const char *name, const char *value);

/* Parses config file and fills confList. confList is dynamically allocated and needs to be freed
 * using crinitFreeConfList() */
int crinitParseConf(crinitConfKvList_t **confList, const char *filename) {
    FILE *cf = fopen(filename, "re");
    if (cf == NULL) {
        crinitErrnoPrint("Could not open \'%s\'.", filename);
        return -1;
    }

    // Read entire file to memory (needed if we want to compare against a signature without TOCTOU race condition)
    char *fileBuf = NULL;
    size_t fileLen;
    if (getdelim(&fileBuf, &fileLen, '\0', cf) == -1) {
        crinitErrnoPrint("Could not read contents of file '%s' to memory.", filename);
        fclose(cf);
        return -1;
    }
    fclose(cf);

    // Check if we must verify the signature of this config file.
    bool sigRequired = CRINIT_CONFIG_DEFAULT_SIGNATURES;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_SIGNATURES, &sigRequired) == -1) {
        crinitErrPrint("Could not retrieve value for global setting if we are to use signatures.");
        free(fileBuf);
        return -1;
    }

    if (sigRequired) {
        crinitErrPrint(
            "Signature checking is not yet implemented. Will carry on as normal. Conf file: '%s' Sig file: '%s.sig'",
            filename, filename);
    }

    // Alloc first element
    if ((*confList = malloc(sizeof(crinitConfKvList_t))) == NULL) {
        crinitErrnoPrint("Could not allocate memory for a ConfKVList.");
        free(fileBuf);
        return -1;
    }
    (*confList)->next = NULL;

    // Parse config from memory with libinih
    crinitIniParserCtx_t parserCtx = {.anchor = *confList, .pList = *confList, .last = *confList};
    int parseResult = ini_parse_string(fileBuf, crinitIniHandler, &parserCtx);

    // Trim the list's tail
    free(parserCtx.last->next);
    parserCtx.last->next = NULL;
    free(fileBuf);

    // Check result
    if (parseResult != 0) {
        if (parseResult == -1) {
            crinitErrPrint("Could not read data from configuration file '%s'.", filename);
        } else if (parseResult == -2) {
            crinitErrPrint("Could not allocate memory during parsing of configuration file '%s'.", filename);
        } else {
            crinitErrPrint("Parser error in configuration file '%s', line %d.", filename, parseResult);
        }
        crinitFreeConfList(*confList);
        *confList = NULL;
        return -1;
    }
    return 0;
}

static int crinitIniHandler(void *parserCtx, const char *section, const char *key, const char *value) {
    CRINIT_PARAM_UNUSED(section);

    crinitIniParserCtx_t *ctx = (crinitIniParserCtx_t *)parserCtx;
    ctx->pList->key = NULL;
    ctx->pList->next = NULL;
    ctx->pList->val = NULL;

    size_t keyLen = strlen(key);
    size_t valLen = strlen(value);

    // Handle legacy array-like keys. This is deprecated and will generate a warning. Current config files using this
    // scheme must be updated. This code block will be removed in one of the next versions.
    const char *brck = strchr(key, '[');
    if (keyLen > 2 && brck != NULL) {
        crinitInfoPrint("Warning: Encountered deprecated use of array brackets in configuration file.");
        //  Decide if key array subscript is empty or not and handle it accordingly
        if (brck[1] == ']') {
            // If this is a beginning of an array declaration, set counter to 0
            if (ctx->last != ctx->pList &&
                (keyLen - 2 != strlen(ctx->last->key) || strncmp(key, ctx->last->key, keyLen - 2) != 0)) {
                ctx->keyArrayCount = 0;
            }
            keyLen -= 2;
        } else {
            char *pEnd = NULL;
            size_t idxLen = strlen(brck);
            // If this is a beginning of an array declaration, set counter to 0
            if (ctx->last != ctx->pList &&
                (keyLen - idxLen != strlen(ctx->last->key) || strncmp(key, ctx->last->key, keyLen - idxLen) != 0)) {
                ctx->keyArrayCount = 0;
            }
            keyLen -= idxLen;

            size_t keyArrIndex = strtoul(brck + 1, &pEnd, 10);
            if (pEnd == brck + 1 || *pEnd != ']') {
                crinitErrPrint("Could not interpret configuration key array subscript: \'%s\'", key);
                return 0;
            }
            if (keyArrIndex != ctx->keyArrayCount) {
                crinitErrPrint("Key array must be specified in order. Subscript '%zu' is unordered.", keyArrIndex);
                return 0;
            }
        }
        ctx->keyArrayCount++;
    }

    // Handle quotes around value.
    const char *mbegin, *mend;
    if (crinitMatchQuotedConfig(value, &mbegin, &mend) == 1) {
        valLen = mend - mbegin;
        value = mbegin;
    }

    // Copy to list
    ctx->pList->key = strndup(key, keyLen);
    ctx->pList->val = strndup(value, valLen);
    if (ctx->pList->key == NULL || ctx->pList->val == NULL) {
        crinitErrnoPrint("Could not allocate memory for a ConfKVList.");
        return 0;
    }

    // Grow list
    ctx->last = ctx->pList;
    if ((ctx->pList->next = malloc(sizeof(crinitConfKvList_t))) == NULL) {
        crinitErrnoPrint("Could not allocate memory for a ConfKVList.");
        return 0;
    }
    ctx->pList = ctx->pList->next;
    return 1;
}

/* Goes through all parts of config list and frees mem */
void crinitFreeConfList(crinitConfKvList_t *confList) {
    if (confList == NULL) {
        return;
    }
    crinitConfKvList_t *last;
    do {
        free(confList->key);
        free(confList->val);
        last = confList;
        confList = confList->next;
        free(last);
    } while (confList != NULL);
}

void crinitFreeArgvArray(char **inArgv) {
    if (inArgv != NULL) {
        // free the backing string
        free(*inArgv);
        // free the outer array
        free(inArgv);
    }
}

int crinitLoadSeriesConf(const char *filename) {
    if (filename == NULL || !crinitIsAbsPath(filename)) {
        crinitErrPrint("Parameters must not be NULL and filename must be an absolute path.");
        return -1;
    }
    crinitConfKvList_t *c;
    if (crinitParseConf(&c, filename) == -1) {
        crinitErrPrint("Could not parse file \'%s\'.", filename);
        return -1;
    }

    bool duplCheckArr[CRINIT_CONFIGS_SIZE] = {false};
    const crinitConfKvList_t *pEntry = c;
    const char *val = NULL;
    while (pEntry != NULL) {
        const crinitConfigMapping_t *scm =
            crinitFindConfigMapping(crinitSeriesCfgMap, crinitSeriesCfgMapSize, pEntry->key);
        if (scm == NULL) {
            crinitInfoPrint("Warning: Unknown configuration key '%s' encountered.", pEntry->key);
        } else {
            val = pEntry->val;
            if ((!scm->arrayLike) && duplCheckArr[scm->config]) {
                crinitErrPrint("Multiple values for non-array like configuration parameter '%s' given.", pEntry->key);
                return -1;
            }

            duplCheckArr[scm->config] = true;
            if (scm->cfgHandler(NULL, val, CRINIT_CONFIG_TYPE_SERIES) == -1) {
                crinitErrPrint("Could not parse configuration parameter '%s' with given value '%s'.", pEntry->key,
                               pEntry->val);
                return -1;
            }
        }
        pEntry = pEntry->next;
    }

    char *taskDir;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_TASKDIR, &taskDir) == -1) {
        crinitErrPrint("Could not retrieve global task directory.");
        return -1;
    }

    if (!duplCheckArr[CRINIT_CONFIG_INCLUDEDIR] &&
        crinitCfgInclDirHandler(NULL, taskDir, CRINIT_CONFIG_TYPE_SERIES) == -1) {
        crinitErrPrint("INCLUDEDIR was not given and trying to set it to the current value of TASKDIR ('%s') failed.",
                       taskDir);
        return -1;
    }

    free(taskDir);
    crinitFreeConfList(c);
    return 0;
}

int crinitLoadTasks(crinitFileSeries_t *series) {
    int res = 0;

    char *taskDir;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_TASKDIR, &taskDir) == -1) {
        crinitErrPrint("Could not retrieve global task directory during task creation.");
        return -1;
    }

    char *taskFileSuffix;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_TASK_FILE_SUFFIX, &taskFileSuffix) == -1) {
        crinitErrPrint("Could not retrieve global task file suffix during task creation.");
        res = -1;
        goto err_task_dir;
    }

    bool taskDirFollowSl;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_TASKDIR_FOLLOW_SYMLINKS, &taskDirFollowSl) == -1) {
        crinitErrPrint("Could not retrieve global taskdir symlink handling during task creation.");
        res = -1;
        goto err_task_suffix;
    }

    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        res = -1;
        goto err_task_suffix;
    }

    char **tasks = globOpts->tasks;
    if (tasks == NULL) {  // No TASKS array given, scan TASKDIR.
        if (crinitFileSeriesFromDir(series, taskDir, taskFileSuffix, taskDirFollowSl) == -1) {
            crinitErrPrint("Could not generate list of tasks from task directory '%s'.", taskDir);
            res = -1;
        }
    } else {  // TASKS taken from config
        if (crinitFileSeriesFromStrArr(series, taskDir, tasks) == -1) {
            crinitErrPrint("Could not generate list of tasks from '%s' option.", CRINIT_CONFIG_KEYSTR_TASKS);
            crinitFreeArgvArray(tasks);
            res = -1;
        }
    }

    if (crinitGlobOptRemit() == -1) {
        crinitErrPrint("Could not release exclusive access of global option storage.");
        res = -1;
    }

err_task_suffix:
    free(taskFileSuffix);

err_task_dir:
    free(taskDir);

    return res;
}
