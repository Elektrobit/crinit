/**
 * @file mock-strcmp.c
 * @brief Implementation of a mock function for strcmp().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-strcmp.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_strcmp(const char *s1, const char *s2) {
    check_expected_ptr(s1);
    check_expected_ptr(s2);
    return mock_type(int);
}
