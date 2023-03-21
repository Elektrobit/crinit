/**
 * @file envset.c
 * @brief Implementation file related to working with environment variable sets.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "envset.h"

#include <stdlib.h>
#include <string.h>

#include "lexers.h"
#include "logio.h"

/**
 * Grows an environment set according to its size increment.
 *
 * Used by EBCL_envSetSet() if not enough space is left in the set.
 *
 * @param es  The environment set to grow, must be initialized.
 *
 * @return  0 on success, -1 otherwise
 */
static int EBCL_envSetGrow(ebcl_EnvSet_t *es);
/**
 * Searches for a given environment variable and returns its index in the set if found.
 *
 * @param es       The environment set to search in.
 * @param envName  Then name of the variable to search for.
 *
 * @return  The index of the variable within ebcl_EnvSet_t::envp if successful, -1 otherwise
 */
static ssize_t EBCL_envSetSearch(const ebcl_EnvSet_t *es, const char *envName);

int EBCL_envSetInit(ebcl_EnvSet_t *es, size_t initSize, size_t sizeIncrement) {
    if (es == NULL) {
        EBCL_errPrint("Input parameter must not be NULL.");
        return -1;
    }
    es->envp = malloc(initSize * sizeof(char *));
    if (es->envp == NULL) {
        EBCL_errnoPrint("Could not allocate memory for environment set.");
        return -1;
    }
    es->allocSz = initSize;
    es->allocInc = sizeIncrement;
    for (size_t i = 0; i < es->allocSz; i++) {
        es->envp[i] = NULL;
    }
    return 0;
}

int EBCL_envSetDestroy(ebcl_EnvSet_t *es) {
    if (es == NULL) {
        EBCL_errPrint("Input parameter must not be NULL.");
        return -1;
    }
    if (es->envp == NULL) {
        return 0;
    }
    for (size_t i = 0; i < es->allocSz && es->envp[i] != NULL; i++) {
        free(es->envp[i]);
        es->envp[i] = NULL;
    }
    free(es->envp);
    es->envp = NULL;
    es->allocSz = 0;
    es->allocInc = 0;
    return 0;
}

int EBCL_envSetDup(ebcl_EnvSet_t *copy, const ebcl_EnvSet_t *orig) {
    if (orig == NULL || orig->envp == NULL || copy == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    if (EBCL_envSetInit(copy, orig->allocSz, orig->allocInc) == -1) {
        EBCL_errPrint("Could not initialize new environment set during duplication.");
        return -1;
    }
    char **pSrc = orig->envp;
    char **pTgt = copy->envp;
    while ((size_t)(pTgt - copy->envp) < (copy->allocSz - 1) && *pSrc != NULL) {
        *pTgt = strdup(*pSrc);
        if (*pTgt == NULL) {
            EBCL_errnoPrint("Could not duplicate string in environment set.");
            EBCL_envSetDestroy(copy);
            return -1;
        }
        pTgt++;
        pSrc++;
    }
    *pTgt = NULL;
    return 0;
}

const char *EBCL_envSetGet(const ebcl_EnvSet_t *es, const char *envName) {
    if (envName == NULL || es == NULL || es->envp == NULL || es->allocSz == 0) {
        return NULL;
    }
    if (strchr(envName, '=') != NULL) {
        EBCL_errPrint("Environment variable names must not contain '='.");
        return NULL;
    }
    ssize_t idx = EBCL_envSetSearch(es, envName);
    if (idx == -1 || es->envp[idx] == NULL) {
        return NULL;
    }
    char *out = strchr(es->envp[idx], '=');
    if (out == NULL) {
        return NULL;
    }
    return out + 1;
}

int EBCL_envSetSet(ebcl_EnvSet_t *es, const char *envName, const char *envVal) {
    if (es == NULL || envName == NULL || envVal == NULL || es->envp == NULL || es->allocSz == 0) {
        EBCL_errPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    size_t newSize = strlen(envName) + strlen(envVal) + 2;
    ssize_t idx = EBCL_envSetSearch(es, envName);
    if (idx == -1) {
        EBCL_errPrint("Could not complete search for environment variable '%s' during variable set.", envName);
    }
    if (es->envp[idx] == NULL) {
        if ((size_t)idx >= es->allocSz - 1) {
            if (EBCL_envSetGrow(es) == -1) {
                EBCL_errPrint("Could not grow environment set of size %zu to size %zu.", es->allocSz,
                              es->allocSz + es->allocInc);
                return -1;
            }
        }
        es->envp[idx] = malloc(newSize * sizeof(char));
        if (es->envp[idx] == NULL) {
            EBCL_errnoPrint("Could not allocate memory for new environment variable.");
            return -1;
        }
    } else {
        if (newSize != strlen(es->envp[idx]) + 1) {
            char *newPtr = realloc(es->envp[idx], newSize * sizeof(char));
            if (newPtr == NULL) {
                EBCL_errnoPrint("Could not reallocate memory to update environment variable.");
                return -1;
            }
            es->envp[idx] = newPtr;
        }
    }
    int n = snprintf(es->envp[idx], newSize, "%s=%s", envName, envVal);
    if (n < 0) {
        EBCL_errPrint("Encoding error during creation/update of environment variable");
        free(es->envp[idx]);
        es->envp[idx] = NULL;
        return -1;
    }

    return 0;
}

int EBCL_envSetParseAndSet(ebcl_EnvSet_t *es, const char *envConf) {
    if (es == NULL || envConf == NULL || es->envp == NULL || es->allocSz == 0) {
        EBCL_errPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    const char *s = envConf, *mbegin = NULL, *mend = NULL;
    bool envKeyFound = false, envValFound = false;
    char *envKey = NULL, *envVal = NULL;
    ebcl_TokenType_t tt;
    do {
        tt = EBCL_envVarOuterLex(&s, &mbegin, &mend);
        switch (tt) {
            case EBCL_TK_ERR:
                EBCL_errPrint("Error while parsing string at '%.*s'\n", (int)(mend - mbegin), mbegin);
                break;
            case EBCL_TK_ENVKEY:
                if (envKeyFound) {
                    EBCL_errPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = EBCL_TK_ERR;
                    break;
                }
                envKey = strndup(mbegin, (size_t)(mend - mbegin));
                if (envKey == NULL) {
                    EBCL_errnoPrint("Could not duplicate environment variable key.");
                    return -1;
                }
                envKeyFound = true;
                break;
            case EBCL_TK_ENVVAL:
                if (!envKeyFound || envValFound) {
                    EBCL_errPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = EBCL_TK_ERR;
                    break;
                }
                envVal = strndup(mbegin, (size_t)(mend - mbegin));
                if (envVal == NULL) {
                    EBCL_errnoPrint("Could not duplicate environment variable value.");
                    free(envKey);
                    return -1;
                }
                envValFound = true;
                break;
            case EBCL_TK_WSPC:
            case EBCL_TK_END:
                break;
            default:
            case EBCL_TK_VAR:
            case EBCL_TK_CPY:
            case EBCL_TK_ESC:
            case EBCL_TK_ESCX:
            case EBCL_TK_UQSTR:
            case EBCL_TK_DQSTR:
                EBCL_errPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                tt = EBCL_TK_ERR;
                break;
        }
    } while (tt != EBCL_TK_END && tt != EBCL_TK_ERR);

    if (!envKeyFound || !envValFound) {
        EBCL_errPrint("Given string does not contain both an environment key and a value.\n");
        free(envVal);
        free(envKey);
        return -1;
    }

    if (tt != EBCL_TK_END) {
        EBCL_errPrint("Parsing of environment string '%s' ended with error.\n", envConf);
        free(envVal);
        free(envKey);
        return -1;
    }

    s = envVal;
    size_t newAllocSz = strlen(s) + 1;
    char *parsedVal = malloc(newAllocSz * sizeof(char));
    if (parsedVal == NULL) {
        EBCL_errnoPrint("Could not allocate buffer to parse environment variable value.");
        free(envKey);
        free(envVal);
        return -1;
    }
    char *pTgt = parsedVal;
    do {
        char *substKey = NULL;
        const char *substVal = NULL;
        size_t substLen = 0;
        char hexBuf[3] = {'\0'};
        char c = '\0';
        size_t escMapIdx = 0;
        tt = EBCL_envVarInnerLex(&s, &mbegin, &mend);
        switch (tt) {
            case EBCL_TK_ERR:
                EBCL_errPrint("Error while parsing string at '%.*s'\n", (int)(mend - mbegin), mbegin);
                break;
            case EBCL_TK_ESC:
                escMapIdx = (size_t)(*(mbegin + 1));
                c = (escMapIdx < sizeof(EBCL_escMap)) ? EBCL_escMap[escMapIdx] : '\0';
                if (c == '\0') {
                    EBCL_errPrint("Unimplemented escape sequence at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = EBCL_TK_ERR;
                    break;
                }
                *pTgt = c;
                pTgt++;
                break;
            case EBCL_TK_ESCX:
                hexBuf[0] = mbegin[0];
                hexBuf[1] = mbegin[1];
                hexBuf[2] = '\0';
                c = (char)strtoul(hexBuf, NULL, 16);
                *pTgt = c;
                pTgt++;
                break;
            case EBCL_TK_VAR:
                substKey = strndup(mbegin, (size_t)(mend - mbegin));
                if (substKey == NULL) {
                    EBCL_errnoPrint("Could not duplicate key of environment variable to expand.");
                    tt = EBCL_TK_ERR;
                    break;
                }
                substVal = EBCL_envSetGet(es, substKey);
                free(substKey);
                // empty substitution
                if (substVal == NULL) {
                    break;
                }
                substLen = strlen(substVal);
                if (substLen > 0) {
                    off_t substOffset = pTgt - parsedVal;
                    newAllocSz += substLen;
                    char *newPtr = realloc(parsedVal, newAllocSz);
                    if (newPtr == NULL) {
                        EBCL_errnoPrint("Could not reallocate memory during environment variable expansion.");
                        tt = EBCL_TK_ERR;
                        break;
                    }
                    parsedVal = newPtr;
                    pTgt = parsedVal + substOffset;
                    pTgt = stpcpy(pTgt, substVal);
                }
                break;
            case EBCL_TK_END:
                *pTgt = '\0';
                break;
            case EBCL_TK_CPY:
            case EBCL_TK_WSPC:
                *pTgt = *mbegin;
                pTgt++;
                break;
            case EBCL_TK_ENVKEY:
            case EBCL_TK_ENVVAL:
            case EBCL_TK_UQSTR:
            case EBCL_TK_DQSTR:
            default:
                EBCL_errPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                tt = EBCL_TK_ERR;
                break;
        }
    } while (tt != EBCL_TK_END && tt != EBCL_TK_ERR);

    free(envVal);

    if (tt != EBCL_TK_END) {
        EBCL_errPrint("Parsing of '%s' ended with an error state.", envConf);
        free(envKey);
        free(parsedVal);
        return -1;
    }

    if (EBCL_envSetSet(es, envKey, parsedVal) == -1) {
        EBCL_errPrint("Could not set environment variable '%s' to '%s'.", envKey, parsedVal);
        free(envKey);
        free(parsedVal);
        return -1;
    }

    free(envKey);
    free(parsedVal);
    return 0;
}

int EBCL_envSetCreateFromConfKvList(ebcl_EnvSet_t *newSet, const ebcl_EnvSet_t *baseSet, const ebcl_ConfKvList_t *c) {
    if (newSet == NULL || c == NULL) {
        EBCL_errPrint("Input config list and output environment set must not be NULL.");
        return -1;
    }

    if (baseSet != NULL && EBCL_envSetDup(newSet, baseSet) == -1) {
        EBCL_errPrint("Could not duplicate base environment set in creation of new merged set.");
        return -1;
    }
    if (baseSet == NULL && EBCL_envSetInit(newSet, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT) == -1) {
        EBCL_errPrint("Could not initialize new environment set.");
        return -1;
    }
    ssize_t numNewEnvs = EBCL_confListKeyGetMaxIdx(c, EBCL_CONFIG_KEYSTR_SETENV) + 1;
    // If EBCL_conflListKeyGetMaxIdx() returns -1, we assume no ENV_SET config lines present.
    for (size_t i = 0; i < (size_t)numNewEnvs; i++) {
        char *val = NULL;
        if (EBCL_confListGetValWithIdx(&val, EBCL_CONFIG_KEYSTR_SETENV, i, c) == -1) {
            EBCL_errPrint("Could not retrieve config key '%s', index %zu from config.", EBCL_CONFIG_KEYSTR_SETENV, i);
            EBCL_envSetDestroy(newSet);
            return -1;
        }
        if (EBCL_envSetParseAndSet(newSet, val) == -1) {
            EBCL_errPrint("Failed to process " EBCL_CONFIG_KEYSTR_SETENV " config item with value '%s'", val);
            EBCL_envSetDestroy(newSet);
            return -1;
        }
    }
    return 0;
}

static ssize_t EBCL_envSetSearch(const ebcl_EnvSet_t *es, const char *envName) {
    if (envName == NULL || es == NULL || es->envp == NULL || es->allocSz == 0) {
        EBCL_errPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    if (strchr(envName, '=') != NULL) {
        EBCL_errPrint("Environment variable names must not contain '='.");
        return -1;
    }
    char **searcher = es->envp;
    while ((size_t)(searcher - es->envp) < es->allocSz && *searcher != NULL) {
        size_t cmpLen = strlen(envName);
        if (strlen(*searcher) > cmpLen && strncmp(*searcher, envName, cmpLen) == 0 && (*searcher)[cmpLen] == '=') {
            return searcher - es->envp;
        }
        searcher++;
    }
    return searcher - es->envp;
}

static int EBCL_envSetGrow(ebcl_EnvSet_t *es) {
    if (es == NULL) {
        EBCL_errPrint("Input parameter must not be NULL.");
        return -1;
    }
    if (es->allocInc == 0) {
        EBCL_errPrint("Could not grow environment set as increment size is 0.");
        return -1;
    }
    size_t newAllocSz = es->allocSz + es->allocInc;
    char **newPtr = realloc(es->envp, newAllocSz * sizeof(char *));
    if (newPtr == NULL) {
        EBCL_errnoPrint("Could not reallocate memory to grow environment set.");
        return -1;
    }
    es->envp = newPtr;
    for (size_t i = es->allocSz; i < newAllocSz; i++) {
        es->envp[i] = NULL;
    }
    es->allocSz = newAllocSz;
    return 0;
}
