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

#include "confconv.h"
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
        crinitErrPrint("Input parameter must not be NULL.");
        return -1;
    }
    es->envp = calloc(initSize, sizeof(*es->envp));
    if (es->envp == NULL) {
        crinitErrnoPrint("Could not allocate memory for environment set.");
        return -1;
    }
    es->allocSz = initSize;
    es->allocInc = sizeIncrement;
    return 0;
}

int EBCL_envSetDestroy(ebcl_EnvSet_t *es) {
    if (es == NULL) {
        crinitErrPrint("Input parameter must not be NULL.");
        return -1;
    }
    if (es->envp == NULL) {
        return 0;
    }
    for (size_t i = 0; es->envp[i] != NULL; i++) {
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
        crinitErrPrint("Input parameters must not be NULL.");
        return -1;
    }
    if (EBCL_envSetInit(copy, orig->allocSz, orig->allocInc) == -1) {
        crinitErrPrint("Could not initialize new environment set during duplication.");
        return -1;
    }
    char **pSrc = orig->envp;
    char **pTgt = copy->envp;
    while (*pSrc != NULL) {
        *pTgt = strdup(*pSrc);
        if (*pTgt == NULL) {
            crinitErrnoPrint("Could not duplicate string in environment set.");
            EBCL_envSetDestroy(copy);
            return -1;
        }
        pTgt++;
        pSrc++;
    }
    return 0;
}

const char *EBCL_envSetGet(const ebcl_EnvSet_t *es, const char *envName) {
    if (envName == NULL || es == NULL || es->envp == NULL || es->allocSz == 0) {
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
        crinitErrPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    size_t newSize = strlen(envName) + strlen(envVal) + 2;
    ssize_t idx = EBCL_envSetSearch(es, envName);
    if (idx == -1) {
        crinitErrPrint("Could not complete search for environment variable '%s' during variable set.", envName);
    } else if (es->envp[idx] == NULL) {
        if ((size_t)idx >= es->allocSz - 1) {
            if (EBCL_envSetGrow(es) == -1) {
                crinitErrPrint("Could not grow environment set of size %zu to size %zu.", es->allocSz,
                              es->allocSz + es->allocInc);
                return -1;
            }
        }
        es->envp[idx] = malloc(newSize * sizeof(char));
        if (es->envp[idx] == NULL) {
            crinitErrnoPrint("Could not allocate memory for new environment variable.");
            return -1;
        }
    } else {
        if (newSize != strlen(es->envp[idx]) + 1) {
            char *newPtr = realloc(es->envp[idx], newSize * sizeof(char));
            if (newPtr == NULL) {
                crinitErrnoPrint("Could not reallocate memory to update environment variable.");
                return -1;
            }
            es->envp[idx] = newPtr;
        }
    }
    int n = snprintf(es->envp[idx], newSize, "%s=%s", envName, envVal);
    if (n < 0) {
        crinitErrPrint("Encoding error during creation/update of environment variable");
        free(es->envp[idx]);
        es->envp[idx] = NULL;
        return -1;
    }

    return 0;
}

int EBCL_envSetCreateFromConfKvList(ebcl_EnvSet_t *newSet, const ebcl_EnvSet_t *baseSet, const ebcl_ConfKvList_t *c) {
    if (newSet == NULL || c == NULL) {
        crinitErrPrint("Input config list and output environment set must not be NULL.");
        return -1;
    }

    if (baseSet != NULL && EBCL_envSetDup(newSet, baseSet) == -1) {
        crinitErrPrint("Could not duplicate base environment set in creation of new merged set.");
        return -1;
    }
    if (baseSet == NULL && EBCL_envSetInit(newSet, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT) == -1) {
        crinitErrPrint("Could not initialize new environment set.");
        return -1;
    }
    ssize_t numNewEnvs = EBCL_confListKeyGetMaxIdx(c, EBCL_CONFIG_KEYSTR_ENV_SET) + 1;
    // If EBCL_conflListKeyGetMaxIdx() returns -1, we assume no ENV_SET config lines present.
    for (size_t i = 0; i < (size_t)numNewEnvs; i++) {
        char *val = NULL;
        if (EBCL_confListGetValWithIdx(&val, EBCL_CONFIG_KEYSTR_ENV_SET, i, c) == -1) {
            crinitErrPrint("Could not retrieve config key '%s', index %zu from config.", EBCL_CONFIG_KEYSTR_ENV_SET, i);
            EBCL_envSetDestroy(newSet);
            return -1;
        }
        if (EBCL_confConvToEnvSetMember(newSet, val) == -1) {
            crinitErrPrint("Failed to process " EBCL_CONFIG_KEYSTR_ENV_SET " config item with value '%s'", val);
            EBCL_envSetDestroy(newSet);
            return -1;
        }
    }
    return 0;
}

static ssize_t EBCL_envSetSearch(const ebcl_EnvSet_t *es, const char *envName) {
    if (envName == NULL || es == NULL || es->envp == NULL || es->allocSz == 0) {
        crinitErrPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    if (strchr(envName, '=') != NULL) {
        crinitErrPrint("Environment variable names must not contain '='.");
        return -1;
    }
    size_t i = 0;
    for (; i < es->allocSz && es->envp[i] != NULL; i++) {
        size_t cmpLen = strlen(envName);
        if (strncmp(es->envp[i], envName, cmpLen) == 0 && es->envp[i][cmpLen] == '=') {
            break;
        }
    }
    return i;
}

static int EBCL_envSetGrow(ebcl_EnvSet_t *es) {
    if (es == NULL) {
        crinitErrPrint("Input parameter must not be NULL.");
        return -1;
    }
    if (es->allocInc == 0) {
        crinitErrPrint("Could not grow environment set as increment size is 0.");
        return -1;
    }
    size_t newAllocSz = es->allocSz + es->allocInc;
    char **newPtr = realloc(es->envp, newAllocSz * sizeof(*newPtr));
    if (newPtr == NULL) {
        crinitErrnoPrint("Could not reallocate memory to grow environment set.");
        return -1;
    }
    es->envp = newPtr;
    memset(&es->envp[es->allocSz], 0, es->allocInc * sizeof(*(es->envp)));
    es->allocSz = newAllocSz;
    return 0;
}
