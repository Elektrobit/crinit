// SPDX-License-Identifier: MIT
/**
 * @file common.c
 * @brief Implementation of common functions not related to other specific features.
 */

#include "common.h"

#include <stdint.h>
#include <stdio.h>

#include "logio.h"

int crinitBinReadAll(uint8_t *buf, size_t n, const char *path) {
    crinitNullCheck(-1, buf, path);
    FILE *inf = fopen(path, "rb");
    if (inf == NULL) {
        crinitErrnoPrint("Could not open '%s' for reading.", path);
        return -1;
    }
    int ret = fread(buf, sizeof(*buf), n, inf);
    if (ferror(inf) || !feof(inf) || ret < 0) {
        crinitErrPrint("Could not read to the end of file of '%s'.", path);
        fclose(inf);
        return -1;
    }
    fclose(inf);
    return ret;
}
