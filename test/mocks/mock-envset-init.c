/**
 * @file mock-envset-init.c
 * @brief Implementation of a mock function for EBCL_envSetInit().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-envset-init.h"

#include <stdio.h>

#include "envset.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_EBCL_envSetInit(ebcl_EnvSet_t *es, size_t initSize, size_t sizeIncrement) {
    fprintf(stderr, "%p %zu %zu\n", (void *)es, initSize, sizeIncrement);
    check_expected(es);
    check_expected(initSize);
    check_expected(sizeIncrement);
    return mock_type(int);
}
