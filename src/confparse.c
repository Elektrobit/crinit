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
#include "elosio.h"
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
    // Alloc first element
    if ((*confList = malloc(sizeof(crinitConfKvList_t))) == NULL) {
        crinitErrnoPrint("Could not allocate memory for a ConfKVList.");
        fclose(cf);
        return -1;
    }
    (*confList)->next = NULL;

    // Parse config ing libinih
    crinitIniParserCtx_t parserCtx = {.anchor = *confList, .pList = *confList, .last = *confList};
    int parseResult = ini_parse_file(cf, crinitIniHandler, &parserCtx);

    // Trim the list's tail
    free(parserCtx.last->next);
    parserCtx.last->next = NULL;
    fclose(cf);

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

static int crinitConfSanityCheck(void) {
    bool useElos = false;
    char *elosServer;
    int elosPort;

    if (crinitGlobOptGet(CRINIT_GLOBOPT_USE_ELOS, &useElos) == -1) {
        crinitErrPrint("Could not recall elos configuration from global options.");
    }

    if (useElos) {
        if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_SERVER, &elosServer) == -1) {
            crinitErrPrint("Could not recall elos server ip from global options.");
        } else {
            if (elosServer == NULL || elosServer[0] == '\0') {
                crinitErrPrint("Elos server configuration missing - disabling elos.");
                crinitGlobOptSetBoolean(CRINIT_GLOBOPT_USE_ELOS, false);
            }

            free(elosServer);
        }

        if (crinitGlobOptGet(CRINIT_GLOBOPT_ELOS_PORT, &elosPort) == -1) {
            crinitErrPrint("Could not recall elos server port from global options.");
        }

        if (elosPort == 0) {
            crinitErrPrint("Elos server port configuration missing - disabling elos.");
            crinitGlobOptSetBoolean(CRINIT_GLOBOPT_USE_ELOS, false);
        }
    }

    return 0;
}

int crinitLoadSeriesConf(crinitFileSeries_t *series, const char *filename) {
    if (series == NULL || filename == NULL || !crinitIsAbsPath(filename)) {
        crinitErrPrint("Parameters must not be NULL and filename must be an absolute path.");
        return -1;
    }
    crinitConfKvList_t *c;
    if (crinitParseConf(&c, filename) == -1) {
        crinitErrPrint("Could not parse file \'%s\'.", filename);
        return -1;
    }

    char **tasks = NULL;
    char *tDir = NULL, *tSuffix = NULL;
    bool tDirSl = CRINIT_CONFIG_DEFAULT_TASKDIR_SYMLINKS;

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

            void *tgt = NULL;
            switch (scm->config) {
                case CRINIT_CONFIG_TASK_FILE_SUFFIX:
                    tgt = &tSuffix;
                    break;
                case CRINIT_CONFIG_TASKDIR:
                    tgt = &tDir;
                    break;
                case CRINIT_CONFIG_TASKDIR_FOLLOW_SYMLINKS:
                    tgt = &tDirSl;
                    break;
                case CRINIT_CONFIG_TASKS:
                    tgt = &tasks;
                    break;
                case CRINIT_CONFIG_ELOS_PORT:
                case CRINIT_CONFIG_ELOS_SERVER:
                case CRINIT_CONFIG_ENV_SET:
                case CRINIT_CONFIG_FILTER_DEFINE:
                case CRINIT_CONFIG_DEBUG:
                case CRINIT_CONFIG_INCLUDE:
                case CRINIT_CONFIG_INCLUDE_SUFFIX:
                case CRINIT_CONFIG_INCLUDEDIR:
                case CRINIT_CONFIG_SHDGRACEP:
                case CRINIT_CONFIG_USE_SYSLOG:
                case CRINIT_CONFIG_USE_ELOS:
                    break;
                case CRINIT_CONFIG_COMMAND:
                case CRINIT_CONFIG_DEPENDS:
                case CRINIT_CONFIG_IOREDIR:
                case CRINIT_CONFIG_NAME:
                case CRINIT_CONFIG_PROVIDES:
                case CRINIT_CONFIG_RESPAWN:
                case CRINIT_CONFIG_RESPAWN_RETRIES:
                case CRINIT_CONFIGS_SIZE:
                default:
                    crinitErrPrint("Unexpected configuration key found in series file: '%s'", scm->configKey);
                    return -1;
            }
            duplCheckArr[scm->config] = true;
            if (scm->cfgHandler(tgt, val, CRINIT_CONFIG_TYPE_SERIES) == -1) {
                crinitErrPrint("Could not parse configuration parameter '%s' with given value '%s'.", pEntry->key,
                               pEntry->val);
                return -1;
            }
        }
        pEntry = pEntry->next;
    }

    if (!duplCheckArr[CRINIT_CONFIG_INCLUDEDIR] &&
        crinitCfgInclDirHandler(NULL, tDir, CRINIT_CONFIG_TYPE_SERIES) == -1) {
        crinitErrPrint("INCLUDEDIR was not given and trying to set it to the current value of TASKDIR ('%s') failed.",
                       tDir);
        return -1;
    }

    if (crinitConfSanityCheck() != 0) {
        crinitErrPrint("Sanity check of global crinit configuration %s failed.", filename);
        return -1;
    }

    if (tasks == NULL) {  // No TASKS array given, scan TASKDIR.
        const char *tDirP = (tDir != NULL) ? tDir : CRINIT_CONFIG_DEFAULT_TASKDIR;
        if (crinitFileSeriesFromDir(series, tDirP, (tSuffix != NULL) ? tSuffix : CRINIT_CONFIG_DEFAULT_TASK_FILE_SUFFIX,
                                    tDirSl) == -1) {
            crinitErrPrint("Could not generate list of tasks from task directory '%s'.", tDirP);
            crinitFreeConfList(c);
            return -1;
        }
    } else {  // TASKS taken from config
        if (crinitFileSeriesFromStrArr(series, (tDir != NULL) ? tDir : CRINIT_CONFIG_DEFAULT_TASKDIR, tasks) == -1) {
            crinitErrPrint("Could not generate list of tasks from '%s' option.", CRINIT_CONFIG_KEYSTR_TASKS);
            crinitFreeConfList(c);
            crinitFreeArgvArray(tasks);
            return -1;
        }
    }

    free(tDir);
    free(tSuffix);
    crinitFreeConfList(c);
    return 0;
}
