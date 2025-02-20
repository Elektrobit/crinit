// SPDX-License-Identifier: MIT
/**
 * @file kcmdline.c
 * @brief Implementation of functions related to working with the Kernel command line.
 */

#include "kcmdline.h"

#include <stdlib.h>

#include "confmap.h"
#include "lexers.h"
#include "logio.h"

int crinitKernelCmdlineParse(const char *cmdlinePath) {
    if (cmdlinePath == NULL) {
        cmdlinePath = CRINIT_KCMDLINE_PATH_DEFAULT;
    }

    char cmdlineBuf[CRINIT_KCMDLINE_MAX_LEN] = {0};
    FILE *cmdlineHdl = fopen(cmdlinePath, "re");
    if (cmdlineHdl == NULL) {
        crinitErrnoPrint("Could not open '%s' for reading.", cmdlinePath);
        return -1;
    }

    if (fgets(cmdlineBuf, sizeof(cmdlineBuf), cmdlineHdl) == NULL) {
        crinitErrnoPrint("Could not read Kernel cmdline from '%s'.", cmdlinePath);
        fclose(cmdlineHdl);
        return -1;
    }
    size_t cmdlineLen = strnlen(cmdlineBuf, sizeof(cmdlineBuf));
    if (cmdlineLen >= sizeof(cmdlineBuf) - 1) {
        crinitErrPrint("Your Kernel cmdline seems to be at least %zu Bytes long. This is too long and not supported.",
                       sizeof(cmdlineBuf) - 1);
        fclose(cmdlineHdl);
        return -1;
    }
    fclose(cmdlineHdl);

    // If the line ends with a newline (that is usually the case), trim it.
    if (cmdlineBuf[cmdlineLen - 1] == '\n') {
        cmdlineBuf[cmdlineLen - 1] = '\0';
    }

    crinitTokenType_t tt;
    const char *s = cmdlineBuf, *keyBegin, *keyEnd, *valBegin, *valEnd;
    enum { TOKEN_START, TOKEN_INNER } parseState = TOKEN_START;

    do {
        tt = crinitKernelCmdlineLex(&s, &keyBegin, &keyEnd, &valBegin, &valEnd);
        switch (tt) {
            case CRINIT_TK_END:
                break;
            case CRINIT_TK_CPY:
                parseState = TOKEN_INNER;
                break;
            case CRINIT_TK_WSPC:
                parseState = TOKEN_START;
                break;
            case CRINIT_TK_VAR:
                if (parseState == TOKEN_START) {
                    parseState = TOKEN_INNER;

                    char *key, *val;
                    crinitDbgInfoPrint("Encountered configuration key '%.*s' with value '%.*s' in Kernel cmdline.",
                                       (int)(keyEnd - keyBegin), keyBegin, (int)(valEnd - valBegin), valBegin);

                    key = strndup(keyBegin, (size_t)(keyEnd - keyBegin));
                    if (key == NULL) {
                        crinitErrnoPrint("Could not duplicate name of Kernel command line variable.");
                        tt = CRINIT_TK_ERR;
                        break;
                    }
                    val = strndup(valBegin, (size_t)(valEnd - valBegin));
                    if (val == NULL) {
                        crinitErrnoPrint("Could not duplicate value of Kernel command line variable.");
                        tt = CRINIT_TK_ERR;
                        free(key);
                        break;
                    }

                    const crinitConfigMapping_t *kccm =
                        crinitFindConfigMapping(crinitKCmdlineCfgMap, crinitKCmdlineCfgMapSize, key);
                    if (kccm == NULL) {
                        crinitInfoPrint(
                            "Warning: Unknown configuration setting 'crinit.%s=%s' encountered on Kernel cmdline.", key,
                            val);
                        free(key);
                        free(val);
                        break;
                    }

                    if (kccm->cfgHandler(NULL, val, CRINIT_CONFIG_TYPE_KCMDLINE) == -1) {
                        crinitErrPrint("Could not interpret option '%s' with value '%s' given on Kernel cmdline.", key,
                                       val);
                        free(key);
                        free(val);
                        return -1;
                    }
                    free(key);
                    free(val);
                }
                break;
            default:
            case CRINIT_TK_UQSTR:
            case CRINIT_TK_DQSTR:
            case CRINIT_TK_ENVKEY:
            case CRINIT_TK_ENVVAL:
            case CRINIT_TK_ESC:
            case CRINIT_TK_ESCX:
            case CRINIT_TK_ERR:
                crinitErrPrint("Parser error at '%.*s'\n", (int)(keyEnd - keyBegin), keyBegin);
                return -1;
        }
    } while (tt != CRINIT_TK_END);

    return 0;
}
