/**
 * @file mock-opendir.c
 * @brief Implementation of a mock function for opendir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-opendir.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
DIR *__wrap_opendir(const char *name) {
    check_expected_ptr(name);
    return mock_ptr_type(DIR *);
}
