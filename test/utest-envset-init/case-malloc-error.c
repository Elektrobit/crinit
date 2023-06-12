/**
 * @file case-malloc-error.c
 * @brief Unit test for EBCL_envSetInit() testing error handling of failed allocation.
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
#include "utest-envset-init.h"

void EBCL_envSetInitTestMallocError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    ebcl_EnvSet_t e = {0};

    expect_value(__wrap_calloc, num, EBCL_ENVSET_INITIAL_SIZE);
    expect_value(__wrap_calloc, size, sizeof(char *));
    will_return(__wrap_calloc, NULL);

    expect_any(__wrap_EBCL_errnoPrintFFL, format);

    assert_int_equal(EBCL_envSetInit(&e, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), -1);
}
