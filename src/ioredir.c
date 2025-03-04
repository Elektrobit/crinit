// SPDX-License-Identifier: MIT
/**
 * @file ioredir.c
 * @brief Implementation of functions related to IO redirection.
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
