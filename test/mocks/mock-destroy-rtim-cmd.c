/**
 * @file mock-destroy-rtim-cmd.c
 * @brief Implementation of a mock function for crinitGlobOptSet().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-destroy-rtim-cmd.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitDestroyRtimCmd(crinitRtimCmd_t *c) {
    check_expected(c);

    return mock_type(int);
}
