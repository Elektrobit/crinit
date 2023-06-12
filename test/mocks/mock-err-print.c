/**
 * @file mock-err-print.c
 * @brief Implementation of a mock function for EBCL_errPrintFFL().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-err-print.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_EBCL_errPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    CRINIT_PARAM_UNUSED(file);
    CRINIT_PARAM_UNUSED(func);
    CRINIT_PARAM_UNUSED(line);
    check_expected(format);
}
