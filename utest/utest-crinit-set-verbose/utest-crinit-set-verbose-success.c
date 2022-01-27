/**
 * @file utest-crinit-set-verbose-success.c
 * @brief Implementation of a unit test for EBCL_crinitSetVerbose(), under the assumption no internal function calls
 *        fail.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
// Rationale: Specific order of includes needed by cmocka.h.

#include <sys/mount.h>

#include "common.h"
#include "crinit-client.h"
#include "mock_globopt_set.h"
#include "utest-crinit-set-verbose.h"

void EBCL_crinitSetVerboseTestSuccess(void **state) {
    EBCL_PARAM_UNUSED(state);
    const bool t = true;
    const bool f = false;

    // case for input == true
    expect_value(__wrap_EBCL_globOptSet, key, EBCL_GLOBOPT_DEBUG);
    expect_memory(__wrap_EBCL_globOptSet, val, &t, sizeof(bool));
    expect_value(__wrap_EBCL_globOptSet, sz, sizeof(bool));
    will_return(__wrap_EBCL_globOptSet, 0);
    assert_int_equal(EBCL_crinitSetVerbose(true), 0);

    // case for input == false
    expect_value(__wrap_EBCL_globOptSet, key, EBCL_GLOBOPT_DEBUG);
    expect_memory(__wrap_EBCL_globOptSet, val, &f, sizeof(bool));
    expect_value(__wrap_EBCL_globOptSet, sz, sizeof(bool));
    will_return(__wrap_EBCL_globOptSet, 0);
    assert_int_equal(EBCL_crinitSetVerbose(false), 0);
}

