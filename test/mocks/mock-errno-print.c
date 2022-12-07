/**
 * @file mock-errno-print.c
 * @brief Implementation of a mock function for EBCL_errnoPrintFFL().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-errno-print.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_EBCL_errnoPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    EBCL_PARAM_UNUSED(file);
    EBCL_PARAM_UNUSED(func);
    EBCL_PARAM_UNUSED(line);
    check_expected(format);
}
