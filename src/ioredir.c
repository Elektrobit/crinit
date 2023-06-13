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

int crinitInitIoRedirFromConfKvList(crinitIoRedir_t *out, const char *key, size_t keyArrIndex,
                                   const ebcl_ConfKvList_t *in) {
    crinitNullCheck(-1, out, key, in);
    char *confVal;
    if (EBCL_confListGetValWithIdx(&confVal, key, keyArrIndex, in) == -1) {
        crinitErrPrint("Could not find %s statement with index %zu in config.", key, keyArrIndex);
        return -1;
    }
    if (EBCL_confConvToIoRedir(out, confVal) == -1) {
        crinitErrPrint("Could not parse IO redirection statement '%s'.", confVal);
        return -1;
    }
    return 0;
}

void crinitDestroyIoRedir(crinitIoRedir_t *ior) {
    free(ior->path);
}

int crinitIoRedirCpy(crinitIoRedir_t *dest, const crinitIoRedir_t *src) {
    crinitNullCheck(-1, dest, src);
    memcpy(dest, src, sizeof(crinitIoRedir_t));
    if (src->path != NULL) {
        dest->path = strdup(src->path);
        if (dest->path == NULL) {
            crinitErrnoPrint("Could not duplicate path string during deep copy of IO redirection definition.");
            return -1;
        }
    }
    return 0;
}

