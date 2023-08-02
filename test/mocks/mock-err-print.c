// SPDX-License-Identifier: MIT
/**
 * @file mock-err-print.c
 * @brief Implementation of a mock function for crinitErrPrintFFL().
 */
#include "mock-err-print.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_crinitErrPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    CRINIT_PARAM_UNUSED(file);
    CRINIT_PARAM_UNUSED(func);
    CRINIT_PARAM_UNUSED(line);
    check_expected(format);
}
