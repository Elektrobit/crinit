/**
 * @file mock-strdup.c
 * @brief Implementation of a mock function for strdup().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-strdup.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
char *__wrap_strdup(const char *s) {
    check_expected_ptr(s);
    return mock_ptr_type(char *);
}
