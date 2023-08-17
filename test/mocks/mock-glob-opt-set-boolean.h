// SPDX-License-Identifier: MIT
/**
 * @file mock-glob-opt-set-boolean.h
 * @brief Header declaring a mock function for crinitGlobOptSetBoolean().
 */
#ifndef __MOCK_GLOBOPT_SET_H__
#define __MOCK_GLOBOPT_SET_H__

#include "globopt.h"

/**
 * Mock function for crinitGlobOptSetBoolean().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API. Otherwise the function
 * is a no-op.
 */
int __wrap_crinitGlobOptSetBoolean(size_t memberOffset, bool val);  // NOLINT(readability-identifier-naming)
                                                                    // Rationale: Naming scheme fixed due to linker
                                                                    // wrapping.

#endif /* __MOCK_GLOBOPT_SET_H__ */
