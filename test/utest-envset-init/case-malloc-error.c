/**
 * @file case-malloc-error.c
 * @brief Unit test for crinitEnvSetInit() testing error handling of failed allocation.
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

void crinitEnvSetInitTestMallocError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitEnvSet_t e = {0};

    expect_value(__wrap_calloc, num, CRINIT_ENVSET_INITIAL_SIZE);
    expect_value(__wrap_calloc, size, sizeof(char *));
    will_return(__wrap_calloc, NULL);

    expect_any(__wrap_crinitErrnoPrintFFL, format);

    assert_int_equal(crinitEnvSetInit(&e, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), -1);
}
