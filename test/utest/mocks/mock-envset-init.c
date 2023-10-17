// SPDX-License-Identifier: MIT
/**
 * @file mock-envset-init.c
 * @brief Implementation of a mock function for crinitEnvSetInit().
 */
#include "mock-envset-init.h"

#include <stdio.h>

#include "envset.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitEnvSetInit(crinitEnvSet_t *es, size_t initSize, size_t sizeIncrement) {
    fprintf(stderr, "%p %zu %zu\n", (void *)es, initSize, sizeIncrement);
    check_expected(es);
    check_expected(initSize);
    check_expected(sizeIncrement);
    return mock_type(int);
}
