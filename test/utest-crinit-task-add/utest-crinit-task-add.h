/**
 * @file utest-crinit-task-add.h
 * @brief Header declaring the unit tests for EBCL_crinitTaskAdd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_CRINIT_TASK_ADD_H__
#define __UTEST_CRINIT_TASK_ADD_H__

#include "rtimcmd.h"
#include <stdint.h>

/**
 * Structure used for \a EBCL_storeRtimCmdContext().
 */
struct EBCL_storeRtimCmdArgs {
    ebcl_RtimCmd_t **ptr;
    ebcl_RtimCmd_t *value;
};

/**
 * Cmocka check function storing the argument value.
 *
 * This function is used to mock a by reference return value that would be written to a pointer. In order to later check
 * the same pointer is used for other calls, this can be used as an expect_check() function that does not actually check
 * the parameter, but saves a copy of the pointer.
 *
 * The parameter types of this function match the Cmocka interface, but will be casted internally as if they were:
 *   int EBCL_storeRtimCmd(const ebcl_RtimCmd_t *value, const ebcl_RtimCmd_t **context);
 *
 * Example usage:
 *   ebcl_RtimCmd_t *EBCL_buildRtimArgCmd;
 *   expect_check(__wrap_EBCL_buildRtimCmd, c, EBCL_storeRtimCmd, &EBCL_buildRtimArgCmd);
 */
int EBCL_storeRtimCmd(const uintmax_t value, const uintmax_t context);
/**
 * Cmocka check function storing the argument value and setting a mocked value.
 *
 * This function is used to mock a by-reference return value by writing a mocked value to the given a pointer. In order
 * to later check the same pointer is used for other calls, this can be used as an expect_check() function that does not
 * actually check the parameter, but saves a copy of the pointer and writes a mocked value to the pointed object.
 *
 * The parameter types of this function match the Cmocka interface, but will be casted internally as if they were:
 *   int EBCL_storeRtimCmd(const ebcl_RtimCmd_t *value, const EBCL_storeRtimCmdArgs *context);
 *
 * The \a context is a casted pointer to a \a EBCL_storeRtimCmdArgs. The argument \a value will be written to the
 * \a context->ptr member, while the member \a context->value will be placed into the object pointed to by \a value.
 *
 * Example usage:
 *   ebcl_RtimCmd_t *EBCL_crinitXferArgRes;
 *   char *EBCL_crinitXferArgResOKArgs[1] = {
 *       EBCL_RTIMCMD_RES_OK
 *   };
 *   ebcl_RtimCmd_t EBCL_crinitXferArgResOK = {
 *       .op = EBCL_RTIMCMD_R_ADDTASK,
 *       .argc = 1,
 *       .args = EBCL_crinitXferArgResOKArgs
 *   };
 *   struct EBCL_storeRtimCmdArgs EBCL_crinitXferArgResContext = {
 *       &EBCL_crinitXferArgRes,
 *       &EBCL_crinitXferArgResOK,
 *   };
 *   expect_check(__wrap_EBCL_crinitXfer, res, EBCL_storeRtimCmdContext, &EBCL_crinitXferArgResContext);
 */
int EBCL_storeRtimCmdContext(const uintmax_t value, const uintmax_t context);
/**
 * Cmocka check function comparing the argument value with the given context.
 *
 * This function is used to check a pointer value passed to a mock function. It uses the pointer value previously saved
 * by \a EBCL_storeRtimCmd(). This has to be done this way, as the arguments to expect_check() are evaluated at test
 * setup before the function under test runs, but the by-reference pointer is only known at runtime.
 *
 * Example usage:
 *   ebcl_RtimCmd_t *EBCL_buildRtimArgCmd;
 *   expect_check(__wrap_EBCL_crinitXfer, cmd, EBCL_checkRtimCmd, &EBCL_buildRtimArgCmd);
 */
int EBCL_checkRtimCmd(const uintmax_t value, const uintmax_t context);

/**
 * Unit test for EBCL_crinitTaskAdd(), successful execution.
 */
void EBCL_crinitTaskAddTestSuccess(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() with confFilePath as NULL.
 */
void EBCL_crinitTaskAddTestConfPathNull(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() with forceDeps as NULL.
 */
void EBCL_crinitTaskAddTestForceDepsNull(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() with forceDeps as empty string.
 */
void EBCL_crinitTaskAddTestForceDepsEmpty(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() testing overwrite is passed on correctly.
 */
void EBCL_crinitTaskAddTestOverwriteBoolToString(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() testing error handling for EBCL_buildRtimCmd().
 */
void EBCL_crinitTaskAddTestBuildRtimCmdError(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() testing error handling for EBCL_crinitXfer().
 */
void EBCL_crinitTaskAddTestCrinitXferError(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() testing error handling for error code response.
 */
void EBCL_crinitTaskAddTestCrinitResponseCodeError(void **state);
/**
 * Unit test for EBCL_crinitTaskAdd() testing error handling for wrong command in response.
 */
void EBCL_crinitTaskAddTestCrinitResponseCmdError(void **state);

#endif /* __UTEST_CRINIT_TASK_ADD_H__ */
