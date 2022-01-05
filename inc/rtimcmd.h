/**
 * @file rtimcmd.h
 * @brief Header related to handling of runtime commands received via the notification/service interface.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __RTIMCMD_H__
#define __RTIMCMD_H__

#include "rtimopmap.h"
#include "taskdb.h"

/**
 * Stack size for the shutdown/reboot handling thread.
 */
#define EBCL_RTIMCMD_SHDN_THREAD_STACK_SIZE (PTHREAD_STACK_MIN + 112 * 1024)

#define EBCL_RTIMCMD_RES_OK "RES_OK"    ///< Value of first argument in a positive (successful) response message.
#define EBCL_RTIMCMD_RES_ERR "RES_ERR"  ///< Value of first argument in a negative (unsuccessful) response message.

/**
 * Structure holding a command or response message with its ebcl_RtimOp_t opcode and arguments array.
 */
typedef struct ebcl_RtimCmd_t {
    ebcl_RtimOp_t op;  ///< The command or response opcode (see rtimopmap.h).
    int argc;          ///< The number of arguments.
    char **args;       ///< String array of arguments.
} ebcl_RtimCmd_t;

/**
 * Create an ebcl_RtimCmd_t from an opcode and an argument list.
 *
 * Will allocate memory for the argument string array which should be freed using EBCL_destroyRtimCmd() when no longer
 * needed.
 *
 * @param c     The ebcl_RtimCmd_t to build.
 * @param op    The opcode of the command or response.
 * @param argc  The number of arguments to the command/response.
 * @param ...   List of \a argc arguments of type `const char*`.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_buildRtimCmd(ebcl_RtimCmd_t *c, ebcl_RtimOp_t op, int argc, ...);
/**
 * Free memory in an ebcl_RtimCmd_t allocated by EBCL_buildRtimCmd() or EBCL_parseRtimCmd().
 *
 * Will free the memory for the argument array in the structure.
 *
 * @param c  The ebcl_RtimCmd_t from which the memory should be freed.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_destroyRtimCmd(ebcl_RtimCmd_t *c);

/**
 * Parses a string into an ebcl_RtimCmd_t.
 *
 * The string must be of the form `<OPCODE_STRING>\nARG1\n...\nARGn`. The mapping of an opcode to a string
 * representation is done in rtimopmap.h. EBCL_rtimCmdToMsgStr() can be used to obtain such a string from an
 * ebcl_RtimCmd_t.
 *
 * Will allocate memory for the argument array inside the output command which should be freed using
 * EBCL_destroyRtimCmd().
 *
 * @param out     The ebcl_RtimCmd_t to create.
 * @param cmdStr  The string to parse.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_parseRtimCmd(ebcl_RtimCmd_t *out, const char *cmdStr);
/**
 * Generates a string representation of an ebcl_RtimCmd_t.
 *
 * The generated string will be in a format parse-able by EBCL_parseRtimCmd(). Memory for the string will be allocated
 * using malloc() and should be freed using free() once no longer used.
 *
 * @param out     Pointer to the output string.
 * @param outLen  Size of the output string including the terminating zero.
 * @param cmd     The ebcl_RtimCmd_t to generate the string from.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_rtimCmdToMsgStr(char **out, size_t *outLen, const ebcl_RtimCmd_t *cmd);
/**
 * Executes an ebcl_RtimCmd_t if it contains a valid command.
 *
 * For the implementations of the different possible commands see rtimcmd.c.
 *
 * @param ctx  Pointer to the ebcl_TaskDB_t the command shall be executed on.
 * @param res  Result/response output.
 * @param cmd  The command to execute.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_execRtimCmd(ebcl_TaskDB_t *ctx, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);

#endif /* __RTIMCMD_H__ */
