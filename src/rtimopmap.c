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

const ebcl_RtimOpMap EBCL_rtimOps[] = {EBCL_genOpMap(EBCL_genOpStruct)};

/**  The number of elements in the generated map. **/
#define EBCL_RTIMOPMAP_ELEMENTS sizeof(EBCL_rtimOps) / sizeof(ebcl_RtimOpMap)

/**
 * Gives the length of a string, delimited either by terminating zero or #EBCL_RTIMCMD_ARGDELIM.
 *
 * @param str  The string. Must be at least one of: null-terminated #EBCL_RTIMCMD_ARGDELIM-terminated.
 *
 * @return  The length of \a str, not including the delimiting character (or the terminating zero).
 */
static inline size_t delimitedStrlen(const char *str);

void EBCL_rtimOpMapDebugPrintAll(void) {
    EBCL_dbgInfoPrint("List of available API Operations:");
    for (size_t i = 0; i < EBCL_RTIMOPMAP_ELEMENTS; i++) {
        EBCL_dbgInfoPrint("opCode: %d, opStr: %s", EBCL_rtimOps[i].opCode, EBCL_rtimOps[i].opStr);
    }
}

int EBCL_rtimOpGetByOpStr(ebcl_RtimOp *out, const char *opStr) {
    if (out == NULL || opStr == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL.");
        return -1;
    }
    size_t n = delimitedStrlen(opStr);
    for (size_t i = 0; i < EBCL_RTIMOPMAP_ELEMENTS; i++) {
        if (n == strlen(EBCL_rtimOps[i].opStr) && strncmp(EBCL_rtimOps[i].opStr, opStr, n) == 0) {
            *out = EBCL_rtimOps[i].opCode;
            return 0;
        }
    }

    EBCL_errPrint("String \'%s\' not mapped to a RtimOp.", opStr);
    return -1;
}

int EBCL_opStrGetByRtimOp(const char **out, ebcl_RtimOp opCode) {
    if (out == NULL) {
        EBCL_errPrint("Return pointer must not be NULL.");
        return -1;
    }

    for (size_t i = 0; i < EBCL_RTIMOPMAP_ELEMENTS; i++) {
        if (EBCL_rtimOps[i].opCode == opCode) {
            *out = EBCL_rtimOps[i].opStr;
            return 0;
        }
    }

    EBCL_errPrint("RtimOp %d not mapped to a string.", opCode);
    return -1;
}

static inline size_t delimitedStrlen(const char *str) {
   size_t len = 0;
   while(*str != '\0' && *str != EBCL_RTIMCMD_ARGDELIM) {
       len++;
       str++;
   }
   return len;
}

