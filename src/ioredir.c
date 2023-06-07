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
#include "confconv.h"
#include "logio.h"

int EBCL_initIoRedirFromConfKvList(ebcl_IoRedir_t *out, const char *key, size_t keyArrIndex,
                                   const ebcl_ConfKvList_t *in) {
    EBCL_nullCheck(-1, out, key, in);
    char *confVal;
    if (EBCL_confListGetValWithIdx(&confVal, key, keyArrIndex, in) == -1) {
        EBCL_errPrint("Could not find %s statement with index %zu in config.", key, keyArrIndex);
        return -1;
    }
    if (EBCL_confConvToIoRedir(out, confVal) == -1) {
        EBCL_errPrint("Could not parse IO redirection statement '%s'.", confVal);
        return -1;
    }
    return 0;
}

void EBCL_destroyIoRedir(ebcl_IoRedir_t *ior) {
    free(ior->path);
}

int EBCL_ioRedirCpy(ebcl_IoRedir_t *dest, const ebcl_IoRedir_t *src) {
    EBCL_nullCheck(-1, dest, src);
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

