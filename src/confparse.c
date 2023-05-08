/**
 * @file confparse.c
 * @brief Implementation of the Config Parser.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "confparse.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "confconv.h"
#include "envset.h"
#include "globopt.h"
#include "ini.h"
#include "ioredir.h"
#include "lexers.h"
#include "logio.h"

/**
 * Struct definition for the parser context used by EBCL_iniHandler()
 */
typedef struct {
    ebcl_ConfKvList_t *anchor;  ///< Anchor pointer to the beginning of the list.
    ebcl_ConfKvList_t *pList;   ///< Running pointer to the element being currently constructed.
    ebcl_ConfKvList_t *last;    ///< Running pointer to the last element just constructed.
    size_t keyArrayCount;       ///< Counter variable for array-like config options.
    size_t envSetCount;         ///< Counter variable for ENV_SET config directives.
    size_t ioRedirCount;        ///< Counter variable for IO_REDIRECT config directives.
} ebcl_IniParserCtx_t;

/**
 * Parser handler for libinih.
 */
static int EBCL_iniHandler(void *parserCtx, const char *section, const char *name, const char *value);

/* Parses config file and fills confList. confList is dynamically allocated and needs to be freed
 * using EBCL_freeConfList() */
int EBCL_parseConf(ebcl_ConfKvList_t **confList, const char *filename) {
    FILE *cf = fopen(filename, "re");
    if (cf == NULL) {
        EBCL_errnoPrint("Could not open \'%s\'.", filename);
        return -1;
    }
    // Alloc first element
    if ((*confList = malloc(sizeof(ebcl_ConfKvList_t))) == NULL) {
        EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
        fclose(cf);
        return -1;
    }
    (*confList)->next = NULL;

    // Parse config ing libinih
    ebcl_IniParserCtx_t parserCtx = {*confList, *confList, *confList, 0, 0, 0};
    int parseResult = ini_parse_file(cf, EBCL_iniHandler, &parserCtx);

    // Trim the list's tail
    free(parserCtx.last->next);
    parserCtx.last->next = NULL;
    fclose(cf);

    // Check result
    if (parseResult != 0) {
        if (parseResult == -1) {
            EBCL_errPrint("Could not read data from configuration file '%s'.", filename);
        } else if (parseResult == -2) {
            EBCL_errPrint("Could not allocate memory during parsing of configuration file '%s'.", filename);
        } else {
            EBCL_errPrint("Parser error in configuration file '%s', line %d.", filename, parseResult);
        }
        EBCL_freeConfList(*confList);
        *confList = NULL;
        return -1;
    }
    return 0;
}

static int EBCL_iniHandler(void *parserCtx, const char *section, const char *key, const char *value) {
    EBCL_PARAM_UNUSED(section);

    ebcl_IniParserCtx_t *ctx = (ebcl_IniParserCtx_t *)parserCtx;
    ctx->pList->key = NULL;
    ctx->pList->next = NULL;
    ctx->pList->val = NULL;
    ctx->pList->keyArrIndex = 0;

    size_t keyLen = strlen(key);
    size_t valLen = strlen(value);

    // Handle array-like keys
    const char *brck = strchr(key, '[');
    if (keyLen > 2 && brck != NULL) {
        //  Decide if key array subscript is empty or not and handle it accordingly
        if (brck[1] == ']') {
            // If this is a beginning of an array declaration, set counter to 0
            if (ctx->last != ctx->pList &&
                (keyLen - 2 != strlen(ctx->last->key) || strncmp(key, ctx->last->key, keyLen - 2) != 0)) {
                ctx->keyArrayCount = 0;
            }
            ctx->pList->keyArrIndex = ctx->keyArrayCount;
            keyLen -= 2;
            ctx->keyArrayCount++;
        } else {
            char *pEnd = NULL;
            ctx->pList->keyArrIndex = strtoul(brck + 1, &pEnd, 10);
            if (pEnd == brck + 1 || *pEnd != ']') {
                EBCL_errPrint("Could not interpret configuration key array subscript: \'%s\'", key);
                return 0;
            }
            keyLen -= strlen(brck);
        }
    } else if (strcmp(key, EBCL_CONFIG_KEYSTR_ENV_SET) == 0) {
        /* Handle ENV_SET (TODO: As we will likely change the whole array handling syntax we should consolidate this
         * with everything else and just differentiate between keys which may be given multiple times and keys which may
         * appear only once. */
        ctx->pList->keyArrIndex = ctx->envSetCount++;
    } else if (strcmp(key, EBCL_CONFIG_KEYSTR_IOREDIR) == 0) {
        // Handle REDIRECT_IO the same way as ENV_SET for now. Same criticism applies.
        ctx->pList->keyArrIndex = ctx->ioRedirCount++;
    }

    // Handle quotes around value.
    const char *mbegin, *mend;
    if (EBCL_matchQuotedConfig(value, &mbegin, &mend) == 1) {
        valLen = mend - mbegin;
        value = mbegin;
    }

    // Copy to list
    ctx->pList->key = strndup(key, keyLen);
    ctx->pList->val = strndup(value, valLen);
    if (ctx->pList->key == NULL || ctx->pList->val == NULL) {
        EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
        return 0;
    }

    // Check for duplicate key
    ebcl_ConfKvList_t *pSrch = ctx->anchor;
    while (pSrch != NULL && pSrch != ctx->pList) {
        if (ctx->pList->keyArrIndex == pSrch->keyArrIndex && strcmp(ctx->pList->key, pSrch->key) == 0) {
            EBCL_errPrint("Found duplicate key \'%s\' (index %zu) in config.", pSrch->key, pSrch->keyArrIndex);
            return 0;
        }
        pSrch = pSrch->next;
    }

    // Grow list
    ctx->last = ctx->pList;
    if ((ctx->pList->next = malloc(sizeof(ebcl_ConfKvList_t))) == NULL) {
        EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
        return 0;
    }
    ctx->pList = ctx->pList->next;
    return 1;
}

/* Goes through all parts of config list and frees mem */
void EBCL_freeConfList(ebcl_ConfKvList_t *confList) {
    if (confList == NULL) {
        return;
    }
    ebcl_ConfKvList_t *last;
    do {
        free(confList->key);
        free(confList->val);
        last = confList;
        confList = confList->next;
        free(last);
    } while (confList != NULL);
}

int EBCL_confListGetValWithIdx(char **val, const char *key, size_t keyArrIndex, const ebcl_ConfKvList_t *c) {
    if (val == NULL || key == NULL || c == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }

    const char *pKey = key;
    while (*pKey != '\0' && isspace(*pKey)) {
        pKey++;
    }

    size_t kLen = strlen(pKey);
    while (kLen > 0 && isspace(pKey[kLen - 1])) {
        kLen--;
    }
    if (kLen == 0) {
        return -1;
    }

    while (c != NULL) {
        if (!strncmp(pKey, c->key, kLen) && c->key[kLen] == '\0' && c->keyArrIndex == keyArrIndex) {
            break;
        }
        c = c->next;
    }
    if (c == NULL) {
        return -1;
    }
    *val = c->val;
    if (*val == NULL) {
        return -1;
    }
    return 0;
}

int EBCL_confListSetValWithIdx(const char *val, const char *key, size_t keyArrIndex, ebcl_ConfKvList_t *c) {
    if (val == NULL || key == NULL || c == NULL) {
        EBCL_errPrint("Input Parameters must not be NULL.");
        return -1;
    }

    const char *pKey = key;
    while (*pKey != '\0' && isspace(*pKey)) {
        pKey++;
    }

    size_t kLen = strlen(pKey);
    while (kLen > 0 && isspace(pKey[kLen - 1])) {
        kLen--;
    }
    if (kLen == 0) {
        return -1;
    }

    while (c != NULL) {
        if (!strncmp(pKey, c->key, kLen) && c->key[kLen] == '\0' && c->keyArrIndex == keyArrIndex) {
            break;
        }
        c = c->next;
    }
    if (c == NULL) {
        return -1;
    }
    free(c->val);
    c->val = strdup(val);
    if (c->val == NULL) {
        return -1;
    }
    return 0;
}

int EBCL_confListExtractBoolean(bool *out, const char *key, bool mandatory, const ebcl_ConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            EBCL_errPrint("Could not get value for mandatory key \"%s\".", key);
            return -1;
        } else {
            // leave *out untouched and return successfully if key is optional
            return 0;
        }
    }
    if (strncmp(val, "YES", 3) == 0) {
        *out = true;
        return 0;
    }
    if (strncmp(val, "NO", 2) == 0) {
        *out = false;
        return 0;
    }
    EBCL_errPrint("Value for \"%s\" is not either \"YES\" or \"NO\" but \"%s\".", key, val);
    return -1;
}

int EBCL_confListExtractSignedInt(int *out, int base, const char *key, bool mandatory, const ebcl_ConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            EBCL_errPrint("Could not get value for mandatory key \"%s\".", key);
            return -1;
        } else {
            // leave *out untouched and return successfully if key is optional
            return 0;
        }
    }
    if (*val == '\0') {
        EBCL_errPrint("Could not convert string to int. String is empty.");
        return -1;
    }
    char *endptr = NULL;
    errno = 0;
    *out = strtol(val, &endptr, base);
    if (errno != 0) {
        EBCL_errnoPrint("Could not convert string to int.");
        return -1;
    }
    if (*endptr != '\0') {
        EBCL_errPrint("Could not convert string to int. Non-numeric characters present in string.");
        return -1;
    }
    return 0;
}

int EBCL_confListExtractUnsignedLL(unsigned long long *out, int base, const char *key, bool mandatory,
                                   const ebcl_ConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            EBCL_errPrint("Could not get value for mandatory key \"%s\".", key);
            return -1;
        } else {
            // leave *out untouched and return successfully if key is optional
            return 0;
        }
    }
    if (*val == '\0') {
        EBCL_errPrint("Could not convert string to unsigned long long. String is empty.");
        return -1;
    }
    char *endptr = NULL;
    errno = 0;
    *out = strtoull(val, &endptr, base);
    if (errno != 0) {
        EBCL_errnoPrint("Could not convert string to unsigned long long.");
        return -1;
    }
    if (*endptr != '\0') {
        EBCL_errPrint("Could not convert string to unsigned long long. Non-numeric characters present in string.");
        return -1;
    }
    return 0;
}

int EBCL_confListExtractArgvArrayWithIdx(int *outArgc, char ***outArgv, const char *key, size_t keyArrIndex,
                                         bool mandatory, const ebcl_ConfKvList_t *in, bool doubleQuoting) {
    if (key == NULL || in == NULL || outArgc == NULL || outArgv == NULL) {
        EBCL_errPrint("\'key\', \'in\', outArgv, and \'outArgc\' parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetValWithIdx(&val, key, keyArrIndex, in) == -1) {
        if (mandatory) {
            EBCL_errPrint("Could not get value for mandatory key \"%s\" (index %zu).", key, keyArrIndex);
            return -1;
        } else {
            // Leave outArgc and outArgv untouched
            return 0;
        }
    }

    *outArgv = EBCL_confConvToStrArr(outArgc, val, doubleQuoting);
    if (outArgv == NULL) {
        EBCL_errPrint("Could not convert configuration value to string array.");
        return -1;
    }
    return 0;
}

void EBCL_freeArgvArray(char **inArgv) {
    if (inArgv != NULL) {
        // free the backing string
        free(*inArgv);
        // free the outer array
        free(inArgv);
    }
}

ssize_t EBCL_confListKeyGetMaxIdx(const ebcl_ConfKvList_t *c, const char *key) {
    if (c == NULL || key == NULL) {
        EBCL_errPrint("Parameters must not be NULL.");
        return -1;
    }

    const char *pKey = key;
    while (*pKey != '\0' && isspace(*pKey)) {
        pKey++;
    }

    size_t kLen = strlen(pKey);
    while (kLen > 0 && isspace(pKey[kLen - 1])) {
        kLen--;
    }
    if (kLen == 0) {
        return -1;
    }

    ssize_t maxIdx = -1;
    while (c != NULL) {
        if (!strncmp(pKey, c->key, kLen) && c->key[kLen] == '\0' && (ssize_t)c->keyArrIndex > maxIdx) {
            maxIdx = (ssize_t)c->keyArrIndex;
        }
        c = c->next;
    }
    return maxIdx;
}

int EBCL_loadSeriesConf(ebcl_FileSeries_t *series, const char *filename) {
    if (series == NULL || !EBCL_isAbsPath(filename)) {
        EBCL_errPrint("Parameters must not be NULL and filename must be an absolute path.");
        return -1;
    }
    ebcl_ConfKvList_t *c;
    if (EBCL_parseConf(&c, filename) == -1) {
        EBCL_errPrint("Could not parse file \'%s\'.", filename);
        return -1;
    }

    char *taskDir = NULL;
    if (EBCL_confListGetVal(&taskDir, EBCL_GLOBOPT_KEYSTR_TASKDIR, c) == -1) {
        EBCL_errPrint("Could not get value for mandatory key \'%s\' in series config \'%s\'.",
                      EBCL_GLOBOPT_KEYSTR_TASKDIR, filename);
        EBCL_freeConfList(c);
        return -1;
    }

    if (EBCL_globOptSetString(EBCL_GLOBOPT_TASKDIR, taskDir) == -1) {
        EBCL_errPrint("Could not store global string option values for \'%s\'.", EBCL_GLOBOPT_KEYSTR_TASKDIR);
        EBCL_freeConfList(c);
        return -1;
    }

    bool followLinks = true;
    if (EBCL_confListExtractBoolean(&followLinks, EBCL_CONFIG_KEYSTR_TASKDIR_SYMLINKS, false, c) == -1) {
        EBCL_errPrint("Could not extract boolean value for key 'TASKDIR_FOLLOW_SYMLINKS'.");
        EBCL_freeConfList(c);
        return -1;
    }

    char *taskFileSuffix = NULL;
    EBCL_confListGetVal(&taskFileSuffix, EBCL_CONFIG_KEYSTR_TASK_FILE_SUFFIX, c);

    char **seriesArr = NULL;
    int seriesLen = 0;
    if (EBCL_confListExtractArgvArray(&seriesLen, &seriesArr, EBCL_CONFIG_KEYSTR_TASKS, false, c, true) == -1) {
        EBCL_errPrint("Could not extract value for key '%s' from '%s'.", EBCL_CONFIG_KEYSTR_TASKS, filename);
        EBCL_freeConfList(c);
        return -1;
    }

    if (seriesArr == NULL) {  // No TASKS array given, scan TASKDIR.
        if (EBCL_fileSeriesFromDir(series, taskDir,
                                   (taskFileSuffix != NULL) ? taskFileSuffix : EBCL_CONFIG_DEFAULT_TASK_FILE_SUFFIX,
                                   followLinks) == -1) {
            EBCL_errPrint("Could not generate list of tasks from task directory '%s'.", taskDir);
            EBCL_freeConfList(c);
            return -1;
        }
    } else {  // TASKS taken from config
        if (EBCL_fileSeriesFromStrArr(series, taskDir, seriesArr) == -1) {
            EBCL_errPrint("Could not generate list of tasks from '%s' option.", EBCL_CONFIG_KEYSTR_TASKS);
            EBCL_freeConfList(c);
            EBCL_freeArgvArray(seriesArr);
            return -1;
        }
    }

    bool confDbg = EBCL_GLOBOPT_DEFAULT_DEBUG;
    if (EBCL_confListExtractBoolean(&confDbg, EBCL_GLOBOPT_KEYSTR_DEBUG, false, c) == -1) {
        EBCL_errPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      EBCL_GLOBOPT_KEYSTR_DEBUG, filename);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }
    if (EBCL_globOptSetBoolean(EBCL_GLOBOPT_DEBUG, &confDbg) == -1) {
        EBCL_errPrint("Could not store global boolean option value for \'%s\'.", EBCL_GLOBOPT_KEYSTR_DEBUG);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }

    bool confUseSyslog = EBCL_GLOBOPT_DEFAULT_USE_SYSLOG;
    if (EBCL_confListExtractBoolean(&confUseSyslog, EBCL_GLOBOPT_KEYSTR_USE_SYSLOG, false, c) == -1) {
        EBCL_errPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      EBCL_GLOBOPT_KEYSTR_USE_SYSLOG, filename);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }
    if (EBCL_globOptSetBoolean(EBCL_GLOBOPT_USE_SYSLOG, &confUseSyslog) == -1) {
        EBCL_errPrint("Could not store global boolean option value for \'%s\'.", EBCL_GLOBOPT_KEYSTR_USE_SYSLOG);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }

    unsigned long long shdnGracePeriodUs = EBCL_GLOBOPT_DEFAULT_SHDGRACEP;
    if (EBCL_confListExtractUnsignedLL(&shdnGracePeriodUs, 10, EBCL_GLOBOPT_KEYSTR_SHDGRACEP, false, c) == -1) {
        EBCL_errPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      EBCL_GLOBOPT_KEYSTR_SHDGRACEP, filename);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }
    if (EBCL_globOptSetUnsignedLL(EBCL_GLOBOPT_SHDGRACEP, &shdnGracePeriodUs) == -1) {
        EBCL_errPrint("Could not store global unsigned long long option values for \'%s\'.",
                      EBCL_GLOBOPT_KEYSTR_SHDGRACEP);
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }

    ebcl_EnvSet_t globEnv;
    if (EBCL_envSetCreateFromConfKvList(&globEnv, NULL, c) == -1) {
        EBCL_errPrint("Could not parse global environment variables from series config.");
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        return -1;
    }

    if (EBCL_globOptSetEnvSet(&globEnv) == -1) {
        EBCL_errPrint("Could not store global environment variable set.");
        EBCL_freeConfList(c);
        EBCL_destroyFileSeries(series);
        EBCL_envSetDestroy(&globEnv);
        return -1;
    }

    EBCL_freeConfList(c);
    EBCL_envSetDestroy(&globEnv);
    return 0;
}
