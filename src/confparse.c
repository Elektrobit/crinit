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

#include "crinit.h"
#include "logio.h"

typedef enum { STATE_UNQUOTED, STATE_QUOTED } ebcl_QuotingState;

/**
 * Trim leading and trailing whitespaces in-place.
 *
 * Will trim any leading and trailing whitespace characters in str by moving the pointer to str
 * and moving the terminating zero.
 *
 * @param str The string to be trimmed in-place, may be NULL in which case trimWhitespace() will
 *            return without doing anything.
 */
static void trimWhitespace(char **str);

/**
 * Swap out character tokens situated between quotation delimeters for a placeholder
 *
 * Will search \a str for any \a token in between two occurences of \a quoteDelim and
 * replace \a token with \a placeholder. Will return an error if an odd number of
 * quotation delimiters is encountered.
 *
 * @param str           The string to be modified.
 * @param quoteDelim    The quotation delimiter.
 * @param token         The character to swap out if encountered in between two quotation delimiters.
 * @param placeholder   The character with which to swap out tokens between two quotation delimiters.
 *
 * @return 0 on success, -1 on error
 */
static int quoteProtectTokens(char *str, char quoteDelim, char token, char placeholder);

/**
 * Restore tokens swapped out by quoteProtectTokens()
 *
 * Will search \a str for all occurences of \a placeholder and swap each out with \a token.
 *
 * @param str          The string to be modified.
 * @param token        The token to be restored.
 * @param placeholder  The placeholder character to be swapped out for \a token.
 */
static void quoteRestoreTokens(char *str, char token, char placeholder);

/* Parses config file and fills confList. confList is dynamically allocated and needs to be freed
 * using EBCL_freeConfList() */
int EBCL_parseConf(ebcl_ConfKvList **confList, const char *filename) {
    char line[4096];  // This should be large if we use lists in a single line
    FILE *cf = fopen(filename, "re");
    if (cf == NULL) {
        EBCL_errnoPrint("Could not open \'%s\'.", filename);
        return -1;
    }
    // Alloc first element
    if ((*confList = malloc(sizeof(ebcl_ConfKvList))) == NULL) {
        EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
        fclose(cf);
        return -1;
    }
    // Parse config line by line
    ebcl_ConfKvList *pList = *confList;
    ebcl_ConfKvList *last = *confList;
    size_t keyArrayCount = 0;
    while (fgets(line, sizeof(line), cf) != NULL) {
        pList->key = NULL;
        pList->next = NULL;
        pList->val = NULL;
        pList->keyArrIndex = 0;

        char *ptr = line;
        // Jump over whitespace at beginning
        while (isspace(*ptr)) {
            ptr++;
        }
        // Empty and comment lines are skipped
        if (*ptr == '#' || *ptr == '\0') {
            continue;
        }

        // Parse line like "KEY = value"
        char *strtok_state = NULL;
        char *sk = strtok_r(ptr, "=\n", &strtok_state);
        char *sv = strtok_r(NULL, "\n", &strtok_state);

        if (sk == NULL || sv == NULL) {
            EBCL_errPrint("Could not parse line: \'%s\'", line);
            fclose(cf);
            EBCL_freeConfList(*confList);
            return -1;
        }

        // Trim leading/trailing whitespaces
        trimWhitespace(&sk);
        trimWhitespace(&sv);

        size_t skLen = strlen(sk);
        size_t svLen = strlen(sv);

        // If the value is enclosed in double quotes, discard those
        if (sv[0] == '\"' && sv[svLen - 1] == '\"') {
            sv[svLen - 1] = '\0';
            sv++;
            svLen -= 2;
        }

        bool autoArray = false;
        // Handle empty key array subscript
        if (skLen > 2 && sk[skLen - 2] == '[' && sk[skLen - 1] == ']') {
            // If this is a beginning of an array declaration, set counter to 0
            if (last != pList && (skLen - 2 != strlen(last->key) || strncmp(sk, last->key, skLen - 2))) {
                keyArrayCount = 0;
            }
            autoArray = true;
            pList->keyArrIndex = keyArrayCount;
            skLen -= 2;
            keyArrayCount++;
            sk[skLen] = '\0';
        }

        // Handle non-empty key array subscript
        if (!autoArray) {
            char *brck = strchr(sk, '[');
            if (brck != NULL) {
                char *pEnd = NULL;
                pList->keyArrIndex = strtoul(brck + 1, &pEnd, 10);
                if (pEnd == brck + 1 || *pEnd != ']') {
                    EBCL_errPrint("Could not interpret configuration key array subscript: \'%s\'", sk);
                    fclose(cf);
                    EBCL_freeConfList(*confList);
                    return -1;
                }
                skLen -= strlen(brck);
                *brck = '\0';
            }
        }

        // Copy to list
        pList->key = malloc(skLen + 1);
        pList->val = malloc(svLen + 1);
        if (pList->key == NULL || pList->val == NULL) {
            EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
            fclose(cf);
            EBCL_freeConfList(*confList);
            return -1;
        }
        memcpy(pList->key, sk, skLen + 1);
        memcpy(pList->val, sv, svLen + 1);

        // Check for duplicate key
        ebcl_ConfKvList *pSrch = *confList;
        while (pSrch != NULL && pSrch != pList) {
            if (pList->keyArrIndex == pSrch->keyArrIndex && strcmp(pList->key, pSrch->key) == 0) {
                EBCL_errPrint("Found duplicate key \'%s\' (index %zu) in config.", pSrch->key, pSrch->keyArrIndex);
                fclose(cf);
                EBCL_freeConfList(*confList);
                return -1;
            }
            pSrch = pSrch->next;
        }

        // Grow list
        last = pList;
        if ((pList->next = malloc(sizeof(ebcl_ConfKvList))) == NULL) {
            EBCL_errnoPrint("Could not allocate memory for a ConfKVList.");
            fclose(cf);
            EBCL_freeConfList(*confList);
            return -1;
        }
        pList = pList->next;
    }
    // Trim the list's tail
    free(last->next);
    last->next = NULL;
    fclose(cf);
    return 0;
}

/* Goes through all parts of config list and frees mem */
void EBCL_freeConfList(ebcl_ConfKvList *confList) {
    if (confList == NULL) {
        return;
    }
    ebcl_ConfKvList *last;
    do {
        free(confList->key);
        free(confList->val);
        last = confList;
        confList = confList->next;
        free(last);
    } while (confList != NULL);
}

int EBCL_confListGetValWithIdx(char **val, const char *key, size_t keyArrIndex, const ebcl_ConfKvList *c) {
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

int EBCL_confListSetValWithIdx(const char *val, const char *key, size_t keyArrIndex, ebcl_ConfKvList *c) {
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

int EBCL_confListExtractBoolean(bool *out, const char *key, const ebcl_ConfKvList *in) {
    if (key == NULL || in == NULL || out == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetVal(&val, key, in) == -1) {
        EBCL_errPrint("Could not get value for \"%s\".", key);
        return -1;
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

int EBCL_confListExtractUnsignedLL(unsigned long long *out, int base, const char *key, const ebcl_ConfKvList *in) {
    if (key == NULL || in == NULL || out == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetVal(&val, key, in) == -1) {
        EBCL_errPrint("Could not get value for \"%s\".", key);
        return -1;
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
                                         const ebcl_ConfKvList *in, bool double_quoting) {
    if (key == NULL || in == NULL || outArgc == NULL || outArgv == NULL) {
        EBCL_errPrint("\'key\', \'in\', outArgv, and \'outArgc\' parameters must not be NULL.");
        return -1;
    }
    char *val = NULL;
    if (EBCL_confListGetValWithIdx(&val, key, keyArrIndex, in) == -1) {
        EBCL_errPrint("Could not get value for key \"%s\" (index %zu).", key, keyArrIndex);
        return -1;
    }

    size_t valLen = strlen(val) + 1;
    char *backbuf = malloc(valLen);
    if (backbuf == NULL) {
        EBCL_errnoPrint("Could not allocate memory for argv backing string.");
        return -1;
    }
    memcpy(backbuf, val, valLen);

    if (double_quoting && (quoteProtectTokens(backbuf, '\"', ' ', -1) == -1)) {
        quoteRestoreTokens(backbuf, ' ', -1);
        EBCL_errPrint("Could not parse quotations in string: \'%s\'", backbuf);
        free(backbuf);
        return -1;
    }

    char *strtokState = NULL;
    char *temp = backbuf;
    *outArgc = 1;
    while (strtok_r(temp, " ", &strtokState) != NULL) {
        temp = NULL;
        (*outArgc)++;
    }
    *outArgv = malloc((*outArgc) * sizeof(char *));
    (*outArgc)--;
    if (*outArgv == NULL) {
        EBCL_errnoPrint("Could not allocate memory for argv-array from config.");
        free(backbuf);
        (*outArgc) = 0;
        return -1;
    }

    // Shortcut if an empty string was given
    if (*outArgc == 0) {
        free(backbuf);
        (*outArgv)[0] = NULL;
        return 0;
    }

    temp = backbuf;
    char **pOut = *outArgv;
    int i = 0;
    while (temp < backbuf + valLen) {
        pOut[i] = temp;
        temp += strlen(temp) + 1;
        i++;
    }
    if (i != *outArgc) {
        EBCL_errPrint("Unexpected number of arguments while trying to generate argv-array from config.");
        free(backbuf);
        free(*outArgv);
        *outArgc = 0;
        *outArgv = NULL;
        return -1;
    }
    pOut[*outArgc] = NULL;

    if (double_quoting) {
        for (int i = 0; i < *outArgc; i++) {
            quoteRestoreTokens(pOut[i], ' ', -1);
            // If the value is enclosed in double quotes, discard those
            if (pOut[i][0] == '\"' && pOut[i][strlen(pOut[i]) - 1] == '\"') {
                pOut[i][strlen(pOut[i]) - 1] = '\0';
                pOut[i]++;
            }
        }
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

ssize_t EBCL_confListKeyGetMaxIdx(const ebcl_ConfKvList *c, const char *key) {
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

    size_t maxIdx = 0;
    while (c != NULL) {
        if (!strncmp(pKey, c->key, kLen) && c->key[kLen] == '\0' && c->keyArrIndex > maxIdx) {
            maxIdx = c->keyArrIndex;
        }
        c = c->next;
    }
    return (ssize_t)maxIdx;
}

static void trimWhitespace(char **str) {
    if (*str == NULL) {
        return;
    }
    // Cut leading whitespaces
    while (isspace(**str)) {
        (*str)++;
    }
    // Find end
    char *end = *str;
    while (*end != '\0') {
        end++;
    }
    // Cut trailing whitespaces while checking not to run out of array backwards
    while ((*str != end) && isspace(*(end - 1))) {
        end--;
    }
    *end = '\0';
}

static int quoteProtectTokens(char *str, char quoteDelim, char token, char placeholder) {
    if (str == NULL) {
        EBCL_errPrint("Input string may not be NULL.");
        return -1;
    }

    ebcl_QuotingState state = STATE_UNQUOTED;
    while (*str != '\0') {
        if (*str == quoteDelim) {
            switch (state) {
                case STATE_UNQUOTED:
                    state = STATE_QUOTED;
                    break;
                case STATE_QUOTED:
                    state = STATE_UNQUOTED;
                    break;
                default:
                    EBCL_errPrint("Reached unexpected state while trying to parse quoted string.");
                    return -1;
            }
        } else if (state == STATE_QUOTED && *str == token) {
            *str = placeholder;
        }
        str++;
    }
    if (state == STATE_QUOTED) {
        EBCL_errPrint("Reached end of string while searching for closing quotation delimiter.");
        return -1;
    }
    return 0;
}

static void quoteRestoreTokens(char *str, char token, char placeholder) {
    if (str == NULL) return;
    while (*str != '\0') {
        if (*str == placeholder) {
            *str = token;
        }
        str++;
    }
}
