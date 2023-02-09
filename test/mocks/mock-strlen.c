/**
 * @file mock-strlen.c
 * @brief Implementation of a mock function for strlen().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-strlen.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
size_t __wrap_strlen(const char *s) {
    check_expected_ptr(s);
    return mock_type(size_t);
}
