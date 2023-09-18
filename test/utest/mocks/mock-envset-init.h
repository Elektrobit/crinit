// SPDX-License-Identifier: MIT
/**
 * @file mock-envset-init.h
 * @brief Header declaring a mock function for crinitEnvSetInit().
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
