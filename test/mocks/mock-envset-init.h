/**
 * @file mock-envset-init.h
 * @brief Header declaring a mock function for crinitEnvSetInit().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_ENVSET_INIT_H__
#define __MOCK_ENVSET_INIT_H__

#include <stddef.h>

#include "envset.h"

/**
 * Mock function for crinitEnvSetInit().
 *
 * Checks that the right parameters are given and returns a preset value.
 */
// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitEnvSetInit(crinitEnvSet_t *es, size_t initSize, size_t sizeIncrement);

#endif /* __MOCK_ENVSET_INIT_H__ */
