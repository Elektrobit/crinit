/**
 * @file case-glob-opt-error.c
 * @brief Implementation of a unit test for crinitClientSetVerbose() with the assumption crinitGlobOptSet() fails.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "crinit-client.h"
#include "mock-glob-opt-set-boolean.h"
#include "unit_test.h"
#include "utest-crinit-set-verbose.h"

void crinitClientSetVerboseTestGlobOptError(void **state) {
    CRINIT_PARAM_UNUSED(state);
    const bool t = true;
    const bool f = false;

    // case for input == true
    expect_value(__wrap_crinitGlobOptSetBoolean, memberOffset, offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_DEBUG));
    expect_value(__wrap_crinitGlobOptSetBoolean, val, t);
    will_return(__wrap_crinitGlobOptSetBoolean, -1);
    assert_int_equal(crinitClientSetVerbose(true), -1);

    // case for input == false
    expect_value(__wrap_crinitGlobOptSetBoolean, memberOffset, offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_DEBUG));
    expect_value(__wrap_crinitGlobOptSetBoolean, val, f);
    will_return(__wrap_crinitGlobOptSetBoolean, -1);
    assert_int_equal(crinitClientSetVerbose(false), -1);
}
