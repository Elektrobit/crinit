// SPDX-License-Identifier: MIT
/**
 * @file rtimopmap.h
 * @brief Header defining the runtime commands available through the notification/service interface.
 */
#ifndef __RTIMOPMAP_H__
#define __RTIMOPMAP_H__

#define CRINIT_RTIMCMD_ARGDELIM '\n'  ///< Delimiting character between arguments of a response or command message.

/**
 * Macro to generate a mapping array for the specified commands.
 */
#define crinitGenOpMap(f)                                                                                   \
    f(ADDTASK) f(ADDSERIES) f(ENABLE) f(DISABLE) f(STOP) f(KILL) f(RESTART) f(NOTIFY) f(STATUS) f(TASKLIST) \
        f(SHUTDOWN) f(GETVER)
/**
 * Macro to generate the opcode enum for crinitGenOpMap().
 *
 * Each command gets a command (`C_`) and a result/response (`R_`) opcode.
 */
#define crinitGenOpEnum(x) CRINIT_RTIMCMD_C_##x, CRINIT_RTIMCMD_R_##x,
/**
 * Macro to generate the opcode-to-string mapping for crinitGenOpMap().
 */
#define crinitGenOpStruct(x) {CRINIT_RTIMCMD_C_##x, "C_" #x}, {CRINIT_RTIMCMD_R_##x, "R_" #x},

/**
 * Enum of the available opcodes, including commands and results/responses.
 */
typedef enum crinitRtimOp_t { crinitGenOpMap(crinitGenOpEnum) } crinitRtimOp_t;

/**
 * Structure holding a single mapping between opcode and string representation.
 */
typedef struct crinitRtimOpMap_t {
    crinitRtimOp_t opCode;  ///< opcode
    const char *opStr;      ///< equivalent string representation
} crinitRtimOpMap_t;

/**
 * The string/opcode map for all opcodes, initialized in rtimopmap.c.
 */
extern const crinitRtimOpMap_t crinitRtimOps[];

/**
 * Given its string representation, find the correct opcode.
 *
 * @param out    Return pointer for the opcode.
 * @param opStr  The string representaiton.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitRtimOpGetByOpStr(crinitRtimOp_t *out, const char *opStr);
/**
 * Given an crinitRtimOp_t opcode, obtain its string representation.
 *
 * The returned point points to static memory that shall not be changed.
 *
 * @param out    Return pointer for the string representation.
 * @param opCode  The opcode.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitOpStrGetByRtimOp(const char **out, crinitRtimOp_t opCode);
/**
 * List available opcodes.
 *
 * Will print the full list of available opcodes in their string representation and the corresponding numerical opcode.
 * Output will only be generated if global option `DEBUG` is `true` as it uses crinitDbgInfoPrint().
 */
void crinitRtimOpMapDebugPrintAll(void);

#endif /* __RTIMOPMAP_H__ */
