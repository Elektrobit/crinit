// SPDX-License-Identifier: MIT
/**
 * @file confconv.c
 * @brief Implementations of conversion operations from configuration values to structured data.
 */

#include "confconv.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "lexers.h"
#include "logio.h"

/**
 * Copies a string while resolving all contained escape sequences.
 *
 * @param dst  Destination string. Must have enough space. Best practice would be "same as \a src" because escape
 *             sequence evaluation can only make the string shorter in this case.
 * @param src  Source string, potentially containing escape sequences.
 * @param end  End pointer, if src should not be read until the terminating null byte.
 *
 * @return  Pointer to end of \a src on success, NULL on failure.
 */
static char *crinitCopyEscaped(char *dst, const char *src, const char *end);

char **crinitConfConvToStrArr(int *numElements, const char *confVal, bool doubleQuoting) {
    crinitNullCheck(NULL, numElements, confVal);
    char *backbuf = calloc(strlen(confVal) + 1, sizeof(*backbuf));
    if (backbuf == NULL) {
        crinitErrnoPrint("Could not allocate memory for argv backing string.");
        return NULL;
    }

    crinitTokenType_t tt;
    const char *s = confVal, *mbegin, *mend;

    // Check how many argv's we're dealing with
    *numElements = 0;
    do {
        tt = crinitArgvLex(&s, &mbegin, &mend, doubleQuoting);
        switch (tt) {
            case CRINIT_TK_WSPC:
            case CRINIT_TK_END:
                break;
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
                (*numElements)++;
                break;
            default:
            case CRINIT_TK_ENVKEY:
            case CRINIT_TK_ENVVAL:
            case CRINIT_TK_VAR:
            case CRINIT_TK_CPY:
            case CRINIT_TK_ESC:
            case CRINIT_TK_ESCX:
            case CRINIT_TK_ERR:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                free(backbuf);
                return NULL;
        }
    } while (tt != CRINIT_TK_END);

    char **outArr = calloc((*numElements + 1), sizeof(*outArr));
    if (outArr == NULL) {
        crinitErrnoPrint("Could not allocate memory for argv-array from config.");
        free(backbuf);
        return NULL;
    }

    // Shortcut if an empty string was given
    if (*numElements == 0) {
        free(backbuf);
        return outArr;
    }

    s = confVal;
    char *runner = backbuf;
    int argc = 0;
    do {
        tt = crinitArgvLex(&s, &mbegin, &mend, doubleQuoting);
        switch (tt) {
            case CRINIT_TK_WSPC:
            case CRINIT_TK_END:
                break;
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
                outArr[argc++] = runner;
                runner = crinitCopyEscaped(runner, mbegin, mend);
                if (runner == NULL) {
                    crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    free(backbuf);
                    free(outArr);
                    return NULL;
                }
                runner++;
                break;
            default:
            case CRINIT_TK_ENVKEY:
            case CRINIT_TK_ENVVAL:
            case CRINIT_TK_VAR:
            case CRINIT_TK_CPY:
            case CRINIT_TK_ESC:
            case CRINIT_TK_ESCX:
            case CRINIT_TK_ERR:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                free(backbuf);
                free(outArr);
                return NULL;
        }
    } while (tt != CRINIT_TK_END);

    if (argc != *numElements) {
        crinitErrPrint("Error trying to parse string array '%s'\n", confVal);
        free(backbuf);
        free(outArr);
        return NULL;
    }

    return outArr;
}

/** Function body for type-generic string-to-integer functions, used internally to deduplicate code. **/
#define crinitConfConvToIntegerFnBody(resType, confVal)                                                   \
    do {                                                                                                 \
        if (*(confVal) == '\0') {                                                                        \
            crinitErrPrint("Could not convert string to int. String is empty.");                          \
            return -1;                                                                                   \
        }                                                                                                \
        char *endptr = NULL;                                                                             \
        errno = 0;                                                                                       \
        (resType) = crinitStrtoGenericInteger(resType, confVal, &endptr, base);                           \
        if (errno != 0) {                                                                                \
            crinitErrnoPrint("Could not convert string to int.");                                         \
            return -1;                                                                                   \
        }                                                                                                \
        if (*endptr != '\0') {                                                                           \
            crinitErrPrint("Could not convert string to int. Non-numeric characters present in string."); \
            return -1;                                                                                   \
        }                                                                                                \
    } while (0)

int crinitConfConvToIntegerI(int *x, const char *confVal, int base) {
    crinitNullCheck(-1, x, confVal);
    crinitConfConvToIntegerFnBody(*x, confVal);
    return 0;
}

int crinitConfConvToIntegerULL(unsigned long long *x, const char *confVal, int base) {
    crinitNullCheck(-1, x, confVal);
    crinitConfConvToIntegerFnBody(*x, confVal);
    return 0;
}

int crinitConfConvToBool(bool *b, const char *confVal) {
    crinitNullCheck(-1, b, confVal);
    if (strcmp(confVal, "YES") == 0) {
        *b = true;
        return 0;
    }
    if (strcmp(confVal, "NO") == 0) {
        *b = false;
        return 0;
    }
    crinitErrPrint("Value for boolean option is not either \"YES\" or \"NO\" but \"%s\".", confVal);
    return -1;
}

int crinitConfConvToIoRedir(crinitIoRedir_t *ior, const char *confVal) {
    crinitNullCheck(-1, ior, confVal);

    memset(ior, 0, sizeof(*ior));
    ior->newFd = -1;
    ior->oldFd = -1;
    ior->oflags = O_TRUNC | O_CREAT;
    ior->mode = 0644;

    int numParams = 0;
    char **confStrArr = crinitConfConvToStrArr(&numParams, confVal, true);
    if (confStrArr == NULL) {
        crinitErrPrint("Could not extract IO redirection parameters from configuration.");
        return -1;
    }

    if (numParams < 2) {
        crinitErrPrint("The IO redirection statement must have at least 2 parameters.");
        crinitFreeArgvArray(confStrArr);
        return -1;
    }

    if (strcmp(confStrArr[0], CRINIT_CONFIG_STDOUT_NAME) == 0) {
        ior->newFd = STDOUT_FILENO;
    } else if (strcmp(confStrArr[0], CRINIT_CONFIG_STDERR_NAME) == 0) {
        ior->newFd = STDERR_FILENO;
    } else if (strcmp(confStrArr[0], CRINIT_CONFIG_STDIN_NAME) == 0) {
        ior->newFd = STDIN_FILENO;
    } else {
        crinitErrPrint("Redirecting other file descriptors than STDOUT, STDERR, or STDIN is not supported.");
        crinitFreeArgvArray(confStrArr);
        return -1;
    }

    if (strcmp(confStrArr[1], CRINIT_CONFIG_STDOUT_NAME) == 0) {
        ior->oldFd = STDOUT_FILENO;
    } else if (strcmp(confStrArr[1], CRINIT_CONFIG_STDERR_NAME) == 0) {
        ior->oldFd = STDERR_FILENO;
    } else if (strcmp(confStrArr[1], CRINIT_CONFIG_STDIN_NAME) == 0) {
        ior->oldFd = STDIN_FILENO;
    } else if (crinitIsAbsPath(confStrArr[1])) {
        ior->path = strdup(confStrArr[1]);
        if (ior->path == NULL) {
            crinitErrnoPrint("Could not duplicate path string during parsing of IO redirection statement.");
            crinitFreeArgvArray(confStrArr);
            return -1;
        }
    } else {
        crinitErrPrint("Redirection target must be a standard file descriptor or an absolute path.");
        crinitFreeArgvArray(confStrArr);
        return -1;
    }

    // The rest of the parameters/flags are only relevant if a file will be opened.
    if (ior->path != NULL) {
        if (numParams > 2) {
            if (strcmp(confStrArr[2], "TRUNCATE") == 0) {
                ior->oflags = O_TRUNC | O_CREAT;
            } else if (strcmp(confStrArr[2], "APPEND") == 0) {
                ior->oflags = O_APPEND | O_CREAT;
            } else if (strcmp(confStrArr[2], "PIPE") == 0) {
                ior->oflags = 0;
                ior->fifo = true;
            } else {
                crinitErrPrint(
                    "Third parameter of redirection statement - if it is given - must be either APPEND, TRUNCATE, or "
                    "PIPE.");
                crinitFreeArgvArray(confStrArr);
                crinitNullify(ior->path);
                return -1;
            }
        }

        if (numParams > 3) {
            char *endPtr;
            ior->mode = crinitStrtoGenericInteger(ior->mode, confStrArr[3], &endPtr, 8);
            if (ior->mode == 0 && (errno != 0 || endPtr == confStrArr[3])) {
                if (errno == 0) {
                    crinitErrPrint(
                        "Encountered token that is not an octal number for file mode in IO redirection statement.");
                } else {
                    crinitErrnoPrint("Could not perform conversion of octal mode digits in IO redirection statement.");
                }
                crinitFreeArgvArray(confStrArr);
                crinitNullify(ior->path);
                return -1;
            } else if (ior->mode > 0777) {
                crinitErrPrint("0%o is not a supported file mode.", ior->mode);
                crinitFreeArgvArray(confStrArr);
                crinitNullify(ior->path);
                return -1;
            }
        }

        if (ior->newFd == STDIN_FILENO) {
            ior->oflags = O_RDONLY;
        } else {
            ior->oflags |= O_WRONLY;
        }
    }

    crinitFreeArgvArray(confStrArr);
    return 0;
}

int crinitConfConvToEnvSetMember(crinitEnvSet_t *es, const char *confVal) {
    if (es == NULL || confVal == NULL || es->envp == NULL || es->allocSz == 0) {
        crinitErrPrint("Input parameters must not be NULL and given environment set must be initialized.");
        return -1;
    }
    const char *s = confVal, *mbegin = NULL, *mend = NULL;
    char *envKey = NULL, *envVal = NULL;
    crinitTokenType_t tt;
    do {
        tt = crinitEnvVarOuterLex(&s, &mbegin, &mend);
        switch (tt) {
            case CRINIT_TK_ERR:
                crinitErrPrint("Error while parsing string at '%.*s'\n", (int)(mend - mbegin), mbegin);
                break;
            case CRINIT_TK_ENVKEY:
                if (envKey != NULL) {
                    crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = CRINIT_TK_ERR;
                    break;
                }
                envKey = strndup(mbegin, (size_t)(mend - mbegin));
                if (envKey == NULL) {
                    crinitErrnoPrint("Could not duplicate environment variable key.");
                    return -1;
                }
                break;
            case CRINIT_TK_ENVVAL:
                if (envKey == NULL || envVal != NULL) {
                    crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = CRINIT_TK_ERR;
                    break;
                }
                envVal = strndup(mbegin, (size_t)(mend - mbegin));
                if (envVal == NULL) {
                    crinitErrnoPrint("Could not duplicate environment variable value.");
                    free(envKey);
                    return -1;
                }
                break;
            case CRINIT_TK_WSPC:
            case CRINIT_TK_END:
                break;
            default:
            case CRINIT_TK_VAR:
            case CRINIT_TK_CPY:
            case CRINIT_TK_ESC:
            case CRINIT_TK_ESCX:
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                tt = CRINIT_TK_ERR;
                break;
        }
    } while (tt != CRINIT_TK_END && tt != CRINIT_TK_ERR);

    if (envKey == NULL || envVal == NULL) {
        crinitErrPrint("Given string does not contain both an environment key and a value.\n");
        free(envVal);
        free(envKey);
        return -1;
    }

    if (tt != CRINIT_TK_END) {
        crinitErrPrint("Parsing of environment string '%s' ended with error.\n", confVal);
        free(envVal);
        free(envKey);
        return -1;
    }

    s = envVal;
    size_t newAllocSz = strlen(s) + 1;
    char *parsedVal = malloc(newAllocSz * sizeof(char));
    if (parsedVal == NULL) {
        crinitErrnoPrint("Could not allocate buffer to parse environment variable value.");
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
        tt = crinitEnvVarInnerLex(&s, &mbegin, &mend);
        switch (tt) {
            case CRINIT_TK_ERR:
                crinitErrPrint("Error while parsing string at '%.*s'\n", (int)(mend - mbegin), mbegin);
                break;
            case CRINIT_TK_ESC:
                escMapIdx = (size_t)(*(mbegin + 1));
                c = (escMapIdx < sizeof(crinitEscMap)) ? crinitEscMap[escMapIdx] : '\0';
                if (c == '\0') {
                    crinitErrPrint("Unimplemented escape sequence at '%.*s'\n", (int)(mend - mbegin), mbegin);
                    tt = CRINIT_TK_ERR;
                    break;
                }
                *pTgt = c;
                pTgt++;
                break;
            case CRINIT_TK_ESCX:
                hexBuf[0] = mbegin[0];
                hexBuf[1] = mbegin[1];
                hexBuf[2] = '\0';
                c = (char)strtoul(hexBuf, NULL, 16);
                *pTgt = c;
                pTgt++;
                break;
            case CRINIT_TK_VAR:
                substKey = strndup(mbegin, (size_t)(mend - mbegin));
                if (substKey == NULL) {
                    crinitErrnoPrint("Could not duplicate key of environment variable to expand.");
                    tt = CRINIT_TK_ERR;
                    break;
                }
                substVal = crinitEnvSetGet(es, substKey);
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
                        crinitErrnoPrint("Could not reallocate memory during environment variable expansion.");
                        tt = CRINIT_TK_ERR;
                        break;
                    }
                    parsedVal = newPtr;
                    pTgt = parsedVal + substOffset;
                    pTgt = stpcpy(pTgt, substVal);
                }
                break;
            case CRINIT_TK_END:
                *pTgt = '\0';
                break;
            case CRINIT_TK_CPY:
            case CRINIT_TK_WSPC:
                *pTgt = *mbegin;
                pTgt++;
                break;
            case CRINIT_TK_ENVKEY:
            case CRINIT_TK_ENVVAL:
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
            default:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(mend - mbegin), mbegin);
                tt = CRINIT_TK_ERR;
                break;
        }
    } while (tt != CRINIT_TK_END && tt != CRINIT_TK_ERR);

    free(envVal);

    if (tt != CRINIT_TK_END) {
        crinitErrPrint("Parsing of '%s' ended with an error state.", confVal);
        free(envKey);
        free(parsedVal);
        return -1;
    }

    if (crinitEnvSetSet(es, envKey, parsedVal) == -1) {
        crinitErrPrint("Could not set environment variable '%s' to '%s'.", envKey, parsedVal);
        free(envKey);
        free(parsedVal);
        return -1;
    }

    free(envKey);
    free(parsedVal);
    return 0;
}

static char *crinitCopyEscaped(char *dst, const char *src, const char *end) {
    crinitNullCheck(NULL, dst, src, end);
    if (src > end) {
        crinitErrPrint("String start pointer must not point behind the end pointer.");
        return NULL;
    }

    char *runner = stpncpy(dst, src, (end - src));
    *runner = '\0';
    runner = dst;
    crinitTokenType_t tt;
    const char *s = runner, *mbegin, *mend;
    do {
        tt = crinitEscLex(&s, &mbegin, &mend);
        if (tt == CRINIT_TK_END) {
            *runner = '\0';
            break;
        } else if (tt == CRINIT_TK_CPY) {
            *runner = *mbegin;
            runner++;
        } else if (tt == CRINIT_TK_ESC) {
            *runner = crinitEscMap[(int)mbegin[1]];
            runner++;
        } else if (tt == CRINIT_TK_ESCX) {
            char match[3] = {mbegin[0], mbegin[1], '\0'};
            *runner = (char)strtol(match, NULL, 16);
            runner++;
        } else {
            crinitErrPrint("Parser error while parsing escape sequences.\n");
            return NULL;
        }

    } while (true);

    return runner;
}

