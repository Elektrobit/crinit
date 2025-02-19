// SPDX-License-Identifier: MIT
/**
 * @file mock-set-err-stream.c
 * @brief Implementation of a mock function for crinitSetErrStream().
 */
#include "mock-set-err-stream.h"

#include "unit_test.h"

void __wrap_crinitSetErrStream(FILE *stream) {  // NOLINT(readability-identifier-naming)
                                                // Rationale: Naming scheme fixed due to linkerw
                                                // wrapping.
    check_expected(stream);
}
