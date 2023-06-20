/**
 * @file mock-glob-opt-set.h
 * @brief Header declaring a mock function for crinitGlobOptSet().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_GLOBOPT_SET_H__
#define __MOCK_GLOBOPT_SET_H__

#include "globopt.h"

/**
 * Mock function for crinitGlobOptSet().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API. Otherwise the function
 * is a no-op.
 */
int __wrap_crinitGlobOptSet(crinitGlobOptKey_t key, const void *val,  // NOLINT(readability-identifier-naming)
                           size_t sz);                              // Rationale: Naming scheme fixed due to linker
                                                                    // wrapping.

#endif /* __MOCK_GLOBOPT_SET_H__ */
