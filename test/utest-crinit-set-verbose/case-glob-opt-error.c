/**
 * @file case-glob-opt-error.c
 * @brief Implementation of a unit test for crinitClientSetVerbose() with the assumption EBCL_globOptSet() fails.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "crinit-client.h"
#include "mock-glob-opt-set.h"
#include "unit_test.h"
#include "utest-crinit-set-verbose.h"

void crinitClientSetVerboseTestGlobOptError(void **state) {
    CRINIT_PARAM_UNUSED(state);
    const bool t = true;
    const bool f = false;

    // case for input == true
    expect_value(__wrap_EBCL_globOptSet, key, EBCL_GLOBOPT_DEBUG);
    expect_memory(__wrap_EBCL_globOptSet, val, &t, sizeof(bool));
    expect_value(__wrap_EBCL_globOptSet, sz, sizeof(bool));
    will_return(__wrap_EBCL_globOptSet, -1);
    assert_int_equal(crinitClientSetVerbose(true), -1);

    // case for input == false
    expect_value(__wrap_EBCL_globOptSet, key, EBCL_GLOBOPT_DEBUG);
    expect_memory(__wrap_EBCL_globOptSet, val, &f, sizeof(bool));
    expect_value(__wrap_EBCL_globOptSet, sz, sizeof(bool));
    will_return(__wrap_EBCL_globOptSet, -1);
    assert_int_equal(crinitClientSetVerbose(false), -1);
}
