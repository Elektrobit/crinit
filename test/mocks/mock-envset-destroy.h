// SPDX-License-Identifier: MIT
/**
 * @file mock-envset-destroy.h
 * @brief Header declaring a mock function for crinitEnvSetDestroy().
 */
#ifndef __MOCK_ENVSET_DESTROY_H__
#define __MOCK_ENVSET_DESTROY_H__

#include "envset.h"

/**
 * Mock function for crinitEnvSetDestroy().
 *
 * Checks that the right parameters are given and returns a preset value.
 */
// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_crinitEnvSetDestroy(crinitEnvSet_t *es);

#endif /* __MOCK_ENVSET_DESTROY_H__ */
