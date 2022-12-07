/**
 * @file mock-calloc.c
 * @brief Implementation of a mock function for calloc().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-calloc.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void *__wrap_calloc(size_t num, size_t size) {
    check_expected(num);
    check_expected(size);
    return mock_ptr_type(void *);
}
