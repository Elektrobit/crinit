// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Implementation of a unit test for crinitClientSetVerbose(), under the assumption no internal function calls
 *        fail.
 */
#include "common.h"
#include "crinit-client.h"
#include "mock-glob-opt-set-boolean.h"
#include "unit_test.h"
#include "utest-crinit-set-verbose.h"

void crinitClientSetVerboseTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);
    const bool t = true;
    const bool f = false;

    // case for input == true
    expect_value(__wrap_crinitGlobOptSetBoolean, memberOffset, offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_DEBUG));
    expect_value(__wrap_crinitGlobOptSetBoolean, val, t);
    will_return(__wrap_crinitGlobOptSetBoolean, 0);
    assert_int_equal(crinitClientSetVerbose(true), 0);

    // case for input == false
    expect_value(__wrap_crinitGlobOptSetBoolean, memberOffset, offsetof(crinitGlobOptStore_t, CRINIT_GLOBOPT_DEBUG));
    expect_value(__wrap_crinitGlobOptSetBoolean, val, f);
    will_return(__wrap_crinitGlobOptSetBoolean, 0);
    assert_int_equal(crinitClientSetVerbose(false), 0);
}
