/**
 * @file mock-envset-destroy.h
 * @brief Header declaring a mock function for crinitEnvSetDestroy().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
