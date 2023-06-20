/**
 * @file rtimopmap.c
 * @brief Implementation of functions related to the runtime command opcode map.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "rtimopmap.h"

#include <stdlib.h>
#include <string.h>

#include "logio.h"

const crinitRtimOpMap_t crinitRtimOps[] = {crinitGenOpMap(crinitGenOpStruct)};

/**  The number of elements in the generated map. **/
#define CRINIT_RTIMOPMAP_ELEMENTS (sizeof(crinitRtimOps) / sizeof(crinitRtimOpMap_t))

/**
 * Gives the length of a string, delimited either by terminating zero or #CRINIT_RTIMCMD_ARGDELIM.
 *
 * @param str  The string. Must be at least one of: null-terminated #CRINIT_RTIMCMD_ARGDELIM-terminated.
 *
 * @return  The length of \a str, not including the delimiting character (or the terminating zero).
 */
static inline size_t EBCL_delimitedStrlen(const char *str);

void crinitRtimOpMapDebugPrintAll(void) {
    crinitDbgInfoPrint("List of available API Operations:");
    for (size_t i = 0; i < CRINIT_RTIMOPMAP_ELEMENTS; i++) {
        crinitDbgInfoPrint("opCode: %d, opStr: %s", crinitRtimOps[i].opCode, crinitRtimOps[i].opStr);
    }
}

int crinitRtimOpGetByOpStr(crinitRtimOp_t *out, const char *opStr) {
    if (out == NULL || opStr == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL.");
        return -1;
    }
    size_t n = EBCL_delimitedStrlen(opStr);
    for (size_t i = 0; i < CRINIT_RTIMOPMAP_ELEMENTS; i++) {
        if (n == strlen(crinitRtimOps[i].opStr) && strncmp(crinitRtimOps[i].opStr, opStr, n) == 0) {
            *out = crinitRtimOps[i].opCode;
            return 0;
        }
    }

    crinitErrPrint("String \'%s\' not mapped to a RtimOp.", opStr);
    return -1;
}

int crinitOpStrGetByRtimOp(const char **out, crinitRtimOp_t opCode) {
    if (out == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }

    for (size_t i = 0; i < CRINIT_RTIMOPMAP_ELEMENTS; i++) {
        if (crinitRtimOps[i].opCode == opCode) {
            *out = crinitRtimOps[i].opStr;
            return 0;
        }
    }

    crinitErrPrint("RtimOp %d not mapped to a string.", opCode);
    return -1;
}

static inline size_t EBCL_delimitedStrlen(const char *str) {
    size_t len = 0;
    while (*str != '\0' && *str != CRINIT_RTIMCMD_ARGDELIM) {
        len++;
        str++;
    }
    return len;
}
