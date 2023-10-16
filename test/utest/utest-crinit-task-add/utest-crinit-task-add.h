// SPDX-License-Identifier: MIT
/**
 * @file utest-crinit-task-add.h
 * @brief Header declaring the unit tests for crinitClientTaskAdd().
 */
#ifndef __UTEST_CRINIT_TASK_ADD_H__
#define __UTEST_CRINIT_TASK_ADD_H__

#include <stdint.h>

#include "rtimcmd.h"

/**
 * Structure used for \a crinitStoreRtimCmdContext().
 */
struct crinitStoreRtimCmdArgs {
    crinitRtimCmd_t **ptr;
    crinitRtimCmd_t *value;
};

/**
 * Cmocka check function storing the argument value.
 *
 * This function is used to mock a by reference return value that would be written to a pointer. In order to later check
 * the same pointer is used for other calls, this can be used as an expect_check() function that does not actually check
 * the parameter, but saves a copy of the pointer.
 *
 * The parameter types of this function match the Cmocka interface, but will be casted internally as if they were:
 *   int crinitStoreRtimCmd(const crinitRtimCmd_t *value, const crinitRtimCmd_t **context);
 *
 * Example usage:
 *   crinitRtimCmd_t *crinitBuildRtimArgCmd;
 *   expect_check(__wrap_crinitBuildRtimCmd, c, crinitStoreRtimCmd, &crinitBuildRtimArgCmd);
 */
int crinitStoreRtimCmd(const uintmax_t value, const uintmax_t context);
/**
 * Cmocka check function storing the argument value and setting a mocked value.
 *
 * This function is used to mock a by-reference return value by writing a mocked value to the given a pointer. In order
 * to later check the same pointer is used for other calls, this can be used as an expect_check() function that does not
 * actually check the parameter, but saves a copy of the pointer and writes a mocked value to the pointed object.
 *
 * The parameter types of this function match the Cmocka interface, but will be casted internally as if they were:
 *   int crinitStoreRtimCmd(const crinitRtimCmd_t *value, const crinitStoreRtimCmdArgs *context);
 *
 * The \a context is a casted pointer to a \a crinitStoreRtimCmdArgs. The argument \a value will be written to the
 * \a context->ptr member, while the member \a context->value will be placed into the object pointed to by \a value.
 *
 * Example usage:
 *   crinitRtimCmd_t *crinitXferArgRes;
 *   char *crinitXferArgResOKArgs[1] = {
 *       CRINIT_RTIMCMD_RES_OK
 *   };
 *   crinitRtimCmd_t crinitXferArgResOK = {
 *       .op = CRINIT_RTIMCMD_R_ADDTASK,
 *       .argc = 1,
 *       .args = crinitXferArgResOKArgs
 *   };
 *   struct crinitStoreRtimCmdArgs crinitXferArgResContext = {
 *       &crinitXferArgRes,
 *       &crinitXferArgResOK,
 *   };
 *   expect_check(__wrap_crinitXfer, res, crinitStoreRtimCmdContext, &crinitXferArgResContext);
 */
int crinitStoreRtimCmdContext(const uintmax_t value, const uintmax_t context);
/**
 * Cmocka check function comparing the argument value with the given context.
 *
 * This function is used to check a pointer value passed to a mock function. It uses the pointer value previously saved
 * by \a crinitStoreRtimCmd(). This has to be done this way, as the arguments to expect_check() are evaluated at test
 * setup before the function under test runs, but the by-reference pointer is only known at runtime.
 *
 * Example usage:
 *   crinitRtimCmd_t *crinitBuildRtimArgCmd;
 *   expect_check(__wrap_crinitXfer, cmd, crinitCheckRtimCmd, &crinitBuildRtimArgCmd);
 */
int crinitCheckRtimCmd(const uintmax_t value, const uintmax_t context);

/**
 * Unit test for crinitClientTaskAdd(), successful execution.
 */
void crinitClientTaskAddTestSuccess(void **state);
/**
 * Unit test for crinitClientTaskAdd() with confFilePath as NULL.
 */
void crinitClientTaskAddTestConfPathNull(void **state);
/**
 * Unit test for crinitClientTaskAdd() with forceDeps as NULL.
 */
void crinitClientTaskAddTestForceDepsNull(void **state);
/**
 * Unit test for crinitClientTaskAdd() with forceDeps as empty string.
 */
void crinitClientTaskAddTestForceDepsEmpty(void **state);
/**
 * Unit test for crinitClientTaskAdd() testing overwrite is passed on correctly.
 */
void crinitClientTaskAddTestOverwriteBoolToString(void **state);
/**
 * Unit test for crinitClientTaskAdd() testing error handling for crinitBuildRtimCmd().
 */
void crinitClientTaskAddTestBuildRtimCmdError(void **state);
/**
 * Unit test for crinitClientTaskAdd() testing error handling for crinitXfer().
 */
void crinitClientTaskAddTestCrinitXferError(void **state);
/**
 * Unit test for crinitClientTaskAdd() testing error handling for error code response.
 */
void crinitClientTaskAddTestCrinitResponseCodeError(void **state);
/**
 * Unit test for crinitClientTaskAdd() testing error handling for wrong command in response.
 */
void crinitClientTaskAddTestCrinitResponseCmdError(void **state);

#endif /* __UTEST_CRINIT_TASK_ADD_H__ */
