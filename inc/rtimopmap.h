/**
 * @file rtimopmap.h
 * @brief Header defining the runtime commands available through the notification/service interface.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __RTIMOPMAP_H__
#define __RTIMOPMAP_H__

#define EBCL_RTIMCMD_ARGDELIM '\n'  ///< Delimiting character between arguments of a response or command message.

/**
 * Macro to generate a mapping array for the specified commands.
 */
#define EBCL_genOpMap(f) \
    f(ADDTASK) f(ADDSERIES) f(ENABLE) f(DISABLE) f(STOP) f(KILL) f(RESTART) f(NOTIFY) f(STATUS) f(SHUTDOWN)
/**
 * Macro to generate the opcode enum for EBCL_genOpMap().
 *
 * Each command gets a command (`C_`) and a result/response (`R_`) opcode.
 */
#define EBCL_genOpEnum(x) EBCL_RTIMCMD_C_##x, EBCL_RTIMCMD_R_##x,
/**
 * Macro to generate the opcode-to-string mapping for EBCL_genOpMap().
 */
#define EBCL_genOpStruct(x) {EBCL_RTIMCMD_C_##x, "C_" #x}, {EBCL_RTIMCMD_R_##x, "R_" #x},

/**
 * Enum of the available opcodes, including commands and results/responses.
 */
typedef enum ebcl_RtimOp_t { EBCL_genOpMap(EBCL_genOpEnum) } ebcl_RtimOp_t;

/**
 * Structure holding a single mapping between opcode and string representation.
 */
typedef struct ebcl_RtimOpMap_t {
    ebcl_RtimOp_t opCode;  ///< opcode
    const char *opStr;     ///< equivalent string representation
} ebcl_RtimOpMap_t;

/**
 * The string/opcode map for all opcodes, initialized in rtimopmap.c.
 */
extern const ebcl_RtimOpMap_t EBCL_rtimOps[];

/**
 * Given its string representation, find the correct opcode.
 *
 * @param out    Return pointer for the opcode.
 * @param opStr  The string representaiton.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_rtimOpGetByOpStr(ebcl_RtimOp_t *out, const char *opStr);
/**
 * Given an ebcl_RtimOp_t opcode, obtain its string representation.
 *
 * The returned point points to static memory that shall not be changed.
 *
 * @param out    Return pointer for the string representation.
 * @param opCode  The opcode.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_opStrGetByRtimOp(const char **out, ebcl_RtimOp_t opCode);
/**
 * List available opcodes.
 *
 * Will print the full list of available opcodes in their string representation and the corresponding numerical opcode.
 * Output will only be generated if global option `DEBUG` is `true` as it uses EBCL_dbgInfoPrint().
 */
void EBCL_rtimOpMapDebugPrintAll(void);

#endif /* __RTIMOPMAP_H__ */
