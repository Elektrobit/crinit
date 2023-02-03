/**
 * @file case-null-input.c
 * @brief Unit test for EBCL_envSetDestroy() with a NULL input.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-envset-destroy.h"

void EBCL_envSetDestroyTestNullInput(void **state) {
    EBCL_PARAM_UNUSED(state);

    expect_any(__wrap_EBCL_errPrintFFL, format);
    assert_int_equal(EBCL_envSetDestroy(NULL), -1);
}
