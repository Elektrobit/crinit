/**
 * @file mock_globopt_set.c
 * @brief Implementation of a mock function for EBCL_globOptSet().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock_globopt_set.h"

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
// Rationale: Specific order of includes needed by cmocka.h.

int __wrap_EBCL_globOptSet(ebcl_GlobOptKey_t key, const void *val,  // NOLINT(readability-identifier-naming)
                           size_t sz) {                             // Rationale: Naming scheme fixed due to linker
                                                                    // wrapping.
    check_expected(key);
    check_expected_ptr(val);
    check_expected(sz);
    return mock_type(int);
}
