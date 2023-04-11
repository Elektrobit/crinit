/**
 * @file ioredir.c
 * @brief Implementation of functions related to IO redirection.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "ioredir.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "logio.h"

int EBCL_initIoRedirFromConfKvList(ebcl_IoRedir_t *out, const char *key, size_t keyArrIndex,
                                   const ebcl_ConfKvList_t *in) {
    if (out == NULL || key == NULL || in == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    out->newFd = -1;
    out->oldFd = -1;
    out->path = NULL;
    out->oflags = O_TRUNC | O_CREAT;
    out->mode = 0644;

    int numParams = 0;
    char **confStrArr = NULL;

    if (EBCL_confListExtractArgvArrayWithIdx(&numParams, &confStrArr, key, keyArrIndex, true, in, true) == -1) {
        EBCL_errPrint("Could not extract IO redirection parameters from task config.");
        return -1;
    }

    if (numParams < 2) {
        EBCL_errPrint("The IO redirection statement must have at least 2 parameters.");
        EBCL_freeArgvArray(confStrArr);
        return -1;
    }

    if (strcmp(confStrArr[0], EBCL_CONFIG_STDOUT_NAME) == 0) {
        out->newFd = STDOUT_FILENO;
    } else if (strcmp(confStrArr[0], EBCL_CONFIG_STDERR_NAME) == 0) {
        out->newFd = STDERR_FILENO;
    } else if (strcmp(confStrArr[0], EBCL_CONFIG_STDIN_NAME) == 0) {
        out->newFd = STDIN_FILENO;
    } else {
        EBCL_errPrint("Redirecting other file descriptors than STDOUT, STDERR, or STDIN is not supported.");
        EBCL_freeArgvArray(confStrArr);
        return -1;
    }

    if (strcmp(confStrArr[1], EBCL_CONFIG_STDOUT_NAME) == 0) {
        out->oldFd = STDOUT_FILENO;
    }
    if (strcmp(confStrArr[1], EBCL_CONFIG_STDERR_NAME) == 0) {
        out->oldFd = STDERR_FILENO;
    }
    if (strcmp(confStrArr[1], EBCL_CONFIG_STDIN_NAME) == 0) {
        out->oldFd = STDIN_FILENO;
    }
    if (out->oldFd != -1) {
        EBCL_freeArgvArray(confStrArr);
        return 0;
    }

    // Assume it's a path if it wasn't either STDOUT, STDERR, or STDIN.
    if (!EBCL_isAbsPath(confStrArr[1])) {
        EBCL_errPrint("Path to file for IO redirection must be absolute.");
        EBCL_freeArgvArray(confStrArr);
        return -1;
    }
    out->path = strdup(confStrArr[1]);
    if (out->path == NULL) {
        EBCL_errnoPrint("Could not duplicate path string during parsing of IO redirection statement.");
        EBCL_freeArgvArray(confStrArr);
        return -1;
    }

    if (numParams > 2) {
        if (strcmp(confStrArr[2], "TRUNCATE") == 0) {
            out->oflags = O_TRUNC | O_CREAT;
        } else if (strcmp(confStrArr[2], "APPEND") == 0) {
            out->oflags = O_APPEND | O_CREAT;
        } else {
            EBCL_errPrint(
                "Third parameter of redirection statement - if it is given - must be either APPEND or TRUNCATE.");
            EBCL_freeArgvArray(confStrArr);
            free(out->path);
            out->path = NULL;
            return -1;
        }
    }

    if (numParams > 3) {
        char *endPtr;
        out->mode = strtoul(confStrArr[3], &endPtr, 8);
        if (out->mode == 0 && (errno != 0 || endPtr == confStrArr[3])) {
            if (errno == 0) {
                EBCL_errPrint(
                    "Encountered token that is not an octal number for file mode in IO redirection statement.");
            } else {
                EBCL_errnoPrint("Could not perform conversion of octal mode digits in IO redirection statement.");
            }
            EBCL_freeArgvArray(confStrArr);
            free(out->path);
            out->path = NULL;
            return -1;
        }
        if (out->mode > 0777) {
            EBCL_errPrint("0%o is not a supported file mode.", out->mode);
            EBCL_freeArgvArray(confStrArr);
            free(out->path);
            out->path = NULL;
            return -1;
        }
    }

    if (out->newFd == STDIN_FILENO) {
        out->oflags = O_RDONLY;
    } else {
        out->oflags |= O_WRONLY;
    }

    EBCL_freeArgvArray(confStrArr);
    return 0;
}

void EBCL_destroyIoRedir(ebcl_IoRedir_t *ior) {
    free(ior->path);
}

int EBCL_ioRedirCpy(ebcl_IoRedir_t *dest, const ebcl_IoRedir_t *src) {
    if (dest == NULL || src == NULL) {
        EBCL_errPrint("Input parameters must not be NULL.");
        return -1;
    }
    memcpy(dest, src, sizeof(ebcl_IoRedir_t));
    if (src->path != NULL) {
        dest->path = strdup(src->path);
        if (dest->path == NULL) {
            EBCL_errnoPrint("Could not duplicate path string during deep copy of IO redirection definition.");
            return -1;
        }
    }
    return 0;
}

