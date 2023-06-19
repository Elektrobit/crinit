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
#define CRINIT_RTIMCMD_SHDN_THREAD_STACK_SIZE (PTHREAD_STACK_MIN + 112 * 1024)

#define CRINIT_RTIMCMD_RES_OK "RES_OK"    ///< Value of first argument in a positive (successful) response message.
#define CRINIT_RTIMCMD_RES_ERR "RES_ERR"  ///< Value of first argument in a negative (unsuccessful) response message.

/**
 * Structure holding a command or response message with its ebcl_RtimOp_t opcode and arguments array.
 */
typedef struct crinitRtimCmd_t {
    ebcl_RtimOp_t op;  ///< The command or response opcode (see rtimopmap.h).
    size_t argc;       ///< The number of arguments.
    char **args;       ///< String array of arguments.
} crinitRtimCmd_t;

/**
 * Create an crinitRtimCmd_t from an opcode and an argument list.
 *
 * Will allocate memory for the argument string array which should be freed using crinitDestroyRtimCmd() when no longer
 * needed.
 *
 * @param c     The crinitRtimCmd_t to build.
 * @param op    The opcode of the command or response.
 * @param argc  The number of arguments to the command/response.
 * @param ...   List of \a argc arguments of type `const char*`.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitBuildRtimCmd(crinitRtimCmd_t *c, ebcl_RtimOp_t op, size_t argc, ...);
/**
 * Free memory in an crinitRtimCmd_t allocated by crinitBuildRtimCmd() or crinitParseRtimCmd().
 *
 * Will free the memory for the argument array in the structure.
 *
 * @param c  The crinitRtimCmd_t from which the memory should be freed.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitDestroyRtimCmd(crinitRtimCmd_t *c);

/**
 * Parses a string into an crinitRtimCmd_t.
 *
 * The string must be of the form `<OPCODE_STRING>\nARG1\n...\nARGn`. The mapping of an opcode to a string
 * representation is done in rtimopmap.h. crinitRtimCmdToMsgStr() can be used to obtain such a string from an
 * crinitRtimCmd_t.
 *
 * Will allocate memory for the argument array inside the output command which should be freed using
 * crinitDestroyRtimCmd().
 *
 * @param out     The crinitRtimCmd_t to create.
 * @param cmdStr  The string to parse.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitParseRtimCmd(crinitRtimCmd_t *out, const char *cmdStr);
/**
 * Generates a string representation of an crinitRtimCmd_t.
 *
 * The generated string will be in a format parse-able by crinitParseRtimCmd(). Memory for the string will be allocated
 * using malloc() and should be freed using free() once no longer used.
 *
 * @param out     Pointer to the output string.
 * @param outLen  Size of the output string including the terminating zero.
 * @param cmd     The crinitRtimCmd_t to generate the string from.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitRtimCmdToMsgStr(char **out, size_t *outLen, const crinitRtimCmd_t *cmd);
/**
 * Executes an crinitRtimCmd_t if it contains a valid command.
 *
 * For the implementations of the different possible commands see rtimcmd.c.
 *
 * @param ctx  Pointer to the crinitTaskDB_t the command shall be executed on.
 * @param res  Result/response output.
 * @param cmd  The command to execute.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitExecRtimCmd(crinitTaskDB_t *ctx, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);

#endif /* __RTIMCMD_H__ */
