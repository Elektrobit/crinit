// SPDX-License-Identifier: MIT
/**
 * @file mock-set-info-stream.c
 * @brief Implementation of a mock function for crinitSetInfoStream().
 */
#include "mock-set-info-stream.h"

#include "unit_test.h"

void __wrap_crinitSetInfoStream(FILE *stream) {  // NOLINT(readability-identifier-naming)
                                                 // Rationale: Naming scheme fixed due to linkerw
                                                 // wrapping.
    check_expected(stream);
}
