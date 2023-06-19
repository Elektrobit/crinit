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
    crinitConfKvList_t *anchor;  ///< Anchor pointer to the beginning of the list.
    crinitConfKvList_t *pList;   ///< Running pointer to the element being currently constructed.
    crinitConfKvList_t *last;    ///< Running pointer to the last element just constructed.
    size_t keyArrayCount;       ///< Counter variable for array-like config options.
    size_t envSetCount;         ///< Counter variable for ENV_SET config directives.
    size_t ioRedirCount;        ///< Counter variable for IO_REDIRECT config directives.
} ebcl_IniParserCtx_t;

/**
 * Parser handler for libinih.
 */
static int EBCL_iniHandler(void *parserCtx, const char *section, const char *name, const char *value);

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
    ebcl_IniParserCtx_t parserCtx = {*confList, *confList, *confList, 0, 0, 0};
    int parseResult = ini_parse_file(cf, EBCL_iniHandler, &parserCtx);

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

static int EBCL_iniHandler(void *parserCtx, const char *section, const char *key, const char *value) {
    CRINIT_PARAM_UNUSED(section);

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
                crinitErrPrint("Could not interpret configuration key array subscript: \'%s\'", key);
                return 0;
            }
            keyLen -= strlen(brck);
        }
    } else if (strcmp(key, CRINIT_CONFIG_KEYSTR_ENV_SET) == 0) {
        /* Handle ENV_SET (TODO: As we will likely change the whole array handling syntax we should consolidate this
         * with everything else and just differentiate between keys which may be given multiple times and keys which may
         * appear only once. */
        ctx->pList->keyArrIndex = ctx->envSetCount++;
    } else if (strcmp(key, CRINIT_CONFIG_KEYSTR_IOREDIR) == 0) {
        // Handle REDIRECT_IO the same way as ENV_SET for now. Same criticism applies.
        ctx->pList->keyArrIndex = ctx->ioRedirCount++;
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

int crinitConfListGetValWithIdx(char **val, const char *key, size_t keyArrIndex, const crinitConfKvList_t *c) {
    if (val == NULL || key == NULL || c == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
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

int crinitConfListSetValWithIdx(const char *val, const char *key, size_t keyArrIndex, crinitConfKvList_t *c) {
    if (val == NULL || key == NULL || c == NULL) {
        crinitErrPrint("Input Parameters must not be NULL.");
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

int crinitConfListExtractBoolean(bool *out, const char *key, bool mandatory, const crinitConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (crinitConfListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            crinitErrPrint("Could not get value for mandatory key \"%s\".", key);
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
    crinitErrPrint("Value for \"%s\" is not either \"YES\" or \"NO\" but \"%s\".", key, val);
    return -1;
}

int crinitConfListExtractSignedInt(int *out, int base, const char *key, bool mandatory, const crinitConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (crinitConfListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            crinitErrPrint("Could not get value for mandatory key \"%s\".", key);
            return -1;
        } else {
            // leave *out untouched and return successfully if key is optional
            return 0;
        }
    }
    if (*val == '\0') {
        crinitErrPrint("Could not convert string to int. String is empty.");
        return -1;
    }
    char *endptr = NULL;
    errno = 0;
    *out = strtol(val, &endptr, base);
    if (errno != 0) {
        crinitErrnoPrint("Could not convert string to int.");
        return -1;
    }
    if (*endptr != '\0') {
        crinitErrPrint("Could not convert string to int. Non-numeric characters present in string.");
        return -1;
    }
    return 0;
}

int crinitConfListExtractUnsignedLL(unsigned long long *out, int base, const char *key, bool mandatory,
                                   const crinitConfKvList_t *in) {
    if (key == NULL || in == NULL || out == NULL) {
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (crinitConfListGetVal(&val, key, in) == -1) {
        if (mandatory) {
            crinitErrPrint("Could not get value for mandatory key \"%s\".", key);
            return -1;
        } else {
            // leave *out untouched and return successfully if key is optional
            return 0;
        }
    }
    if (*val == '\0') {
        crinitErrPrint("Could not convert string to unsigned long long. String is empty.");
        return -1;
    }
    char *endptr = NULL;
    errno = 0;
    *out = strtoull(val, &endptr, base);
    if (errno != 0) {
        crinitErrnoPrint("Could not convert string to unsigned long long.");
        return -1;
    }
    if (*endptr != '\0') {
        crinitErrPrint("Could not convert string to unsigned long long. Non-numeric characters present in string.");
        return -1;
    }
    return 0;
}

int crinitConfListExtractArgvArrayWithIdx(int *outArgc, char ***outArgv, const char *key, size_t keyArrIndex,
                                         bool mandatory, const crinitConfKvList_t *in, bool doubleQuoting) {
    if (key == NULL || in == NULL || outArgc == NULL || outArgv == NULL) {
        crinitErrPrint("\'key\', \'in\', outArgv, and \'outArgc\' parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (crinitConfListGetValWithIdx(&val, key, keyArrIndex, in) == -1) {
        if (mandatory) {
            crinitErrPrint("Could not get value for mandatory key \"%s\" (index %zu).", key, keyArrIndex);
            return -1;
        } else {
            // Leave outArgc and outArgv untouched
            return 0;
        }
    }

    *outArgv = crinitConfConvToStrArr(outArgc, val, doubleQuoting);
    if (outArgv == NULL) {
        crinitErrPrint("Could not convert configuration value to string array.");
        return -1;
    }
    return 0;
}

void crinitFreeArgvArray(char **inArgv) {
    if (inArgv != NULL) {
        // free the backing string
        free(*inArgv);
        // free the outer array
        free(inArgv);
    }
}

ssize_t crinitConfListKeyGetMaxIdx(const crinitConfKvList_t *c, const char *key) {
    if (c == NULL || key == NULL) {
        crinitErrPrint("Parameters must not be NULL.");
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

    char *taskDir = NULL;
    if (crinitConfListGetVal(&taskDir, CRINIT_CONFIG_KEYSTR_TASKDIR, c) == -1) {
        crinitErrPrint("Could not get value for mandatory key \'%s\' in series config \'%s\'.",
                      CRINIT_CONFIG_KEYSTR_TASKDIR, filename);
        crinitFreeConfList(c);
        return -1;
    }

    if (crinitGlobOptSetString(CRINIT_GLOBOPT_TASKDIR, taskDir) == -1) {
        crinitErrPrint("Could not store global string option values for '%s'.", CRINIT_CONFIG_KEYSTR_TASKDIR);
        crinitFreeConfList(c);
        return -1;
    }

    bool followLinks = true;
    if (crinitConfListExtractBoolean(&followLinks, CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS, false, c) == -1) {
        crinitErrPrint("Could not extract boolean value for key '%s'.", CRINIT_CONFIG_KEYSTR_TASKDIR_SYMLINKS);
        crinitFreeConfList(c);
        return -1;
    }

    char *fileSuffix = NULL;
    crinitConfListGetVal(&fileSuffix, CRINIT_CONFIG_KEYSTR_TASK_FILE_SUFFIX, c);

    char **seriesArr = NULL;
    int seriesLen = 0;
    if (crinitConfListExtractArgvArray(&seriesLen, &seriesArr, CRINIT_CONFIG_KEYSTR_TASKS, false, c, true) == -1) {
        crinitErrPrint("Could not extract value for key '%s' from '%s'.", CRINIT_CONFIG_KEYSTR_TASKS, filename);
        crinitFreeConfList(c);
        return -1;
    }

    if (seriesArr == NULL) {  // No TASKS array given, scan TASKDIR.
        if (crinitFileSeriesFromDir(series, taskDir,
                                   (fileSuffix != NULL) ? fileSuffix : CRINIT_CONFIG_DEFAULT_TASK_FILE_SUFFIX,
                                   followLinks) == -1) {
            crinitErrPrint("Could not generate list of tasks from task directory '%s'.", taskDir);
            crinitFreeConfList(c);
            return -1;
        }
    } else {  // TASKS taken from config
        if (crinitFileSeriesFromStrArr(series, taskDir, seriesArr) == -1) {
            crinitErrPrint("Could not generate list of tasks from '%s' option.", CRINIT_CONFIG_KEYSTR_TASKS);
            crinitFreeConfList(c);
            crinitFreeArgvArray(seriesArr);
            return -1;
        }
    }

    char *inclDir = taskDir;
    crinitConfListGetVal(&inclDir, CRINIT_CONFIG_KEYSTR_INCLDIR, c);
    if (crinitGlobOptSetString(CRINIT_GLOBOPT_INCLDIR, inclDir) == -1) {
        crinitErrPrint("Could not store global string option values for '%s'.", CRINIT_CONFIG_KEYSTR_INCLDIR);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    if (crinitConfListGetVal(&fileSuffix, CRINIT_CONFIG_KEYSTR_INCL_SUFFIX, c) == 0 &&
        crinitGlobOptSetString(CRINIT_GLOBOPT_INCL_SUFFIX, fileSuffix) == -1) {
        crinitErrPrint("Could not store global string option values for '%s'.", CRINIT_CONFIG_KEYSTR_INCL_SUFFIX);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    bool confDbg = CRINIT_CONFIG_DEFAULT_DEBUG;
    if (crinitConfListExtractBoolean(&confDbg, CRINIT_CONFIG_KEYSTR_DEBUG, false, c) == -1) {
        crinitErrPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      CRINIT_CONFIG_KEYSTR_DEBUG, filename);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }
    if (crinitGlobOptSetBoolean(CRINIT_GLOBOPT_DEBUG, &confDbg) == -1) {
        crinitErrPrint("Could not store global boolean option value for \'%s\'.", CRINIT_CONFIG_KEYSTR_DEBUG);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    bool confUseSyslog = CRINIT_CONFIG_DEFAULT_USE_SYSLOG;
    if (crinitConfListExtractBoolean(&confUseSyslog, CRINIT_CONFIG_KEYSTR_USE_SYSLOG, false, c) == -1) {
        crinitErrPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      CRINIT_CONFIG_KEYSTR_USE_SYSLOG, filename);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }
    if (crinitGlobOptSetBoolean(CRINIT_GLOBOPT_USE_SYSLOG, &confUseSyslog) == -1) {
        crinitErrPrint("Could not store global boolean option value for \'%s\'.", CRINIT_CONFIG_KEYSTR_USE_SYSLOG);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    unsigned long long shdnGracePeriodUs = CRINIT_CONFIG_DEFAULT_SHDGRACEP;
    if (crinitConfListExtractUnsignedLL(&shdnGracePeriodUs, 10, CRINIT_CONFIG_KEYSTR_SHDGRACEP, false, c) == -1) {
        crinitErrPrint("Failed to search for non-mandatory key \'%s\' in series config \'%s\'.",
                      CRINIT_CONFIG_KEYSTR_SHDGRACEP, filename);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }
    if (crinitGlobOptSetUnsignedLL(CRINIT_GLOBOPT_SHDGRACEP, &shdnGracePeriodUs) == -1) {
        crinitErrPrint("Could not store global unsigned long long option values for \'%s\'.",
                      CRINIT_CONFIG_KEYSTR_SHDGRACEP);
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    crinitEnvSet_t globEnv;
    if (crinitEnvSetCreateFromConfKvList(&globEnv, NULL, c) == -1) {
        crinitErrPrint("Could not parse global environment variables from series config.");
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        return -1;
    }

    if (crinitGlobOptSetEnvSet(&globEnv) == -1) {
        crinitErrPrint("Could not store global environment variable set.");
        crinitFreeConfList(c);
        crinitDestroyFileSeries(series);
        crinitEnvSetDestroy(&globEnv);
        return -1;
    }

    crinitFreeConfList(c);
    crinitEnvSetDestroy(&globEnv);
    return 0;
}
