/**
 * @file mock-envset-destroy.c
 * @brief Implementation of a mock function for EBCL_envSetDestroy().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-envset-destroy.h"

#include "envset.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_EBCL_envSetDestroy(ebcl_EnvSet_t *es) {
    check_expected_ptr(es);
    return mock_type(int);
}
