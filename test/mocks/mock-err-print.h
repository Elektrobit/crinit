// SPDX-License-Identifier: MIT
/**
 * @file mock-err-print.h
 * @brief Header declaring a mock function for crinitErrPrintFFL().
 */
#ifndef __MOCK_ERRNO_PRINT_H__
#define __MOCK_ERRNO_PRINT_H__

/**
 * Mock function for crinitErrPrintFFL().
 *
 * Checks the format string and ignores everything else.
 */
// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
void __wrap_crinitErrPrintFFL(const char *file, const char *func, int line, const char *format, ...)
    __attribute__((format(printf, 4, 5)));

#endif /* __MOCK_ERRNO_PRINT_H__ */
