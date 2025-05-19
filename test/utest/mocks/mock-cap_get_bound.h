// SPDX-License-Identifier: MIT
/**
 * @file mock-cap_get_bound.h
 * @brief Header declaring a mock function for cap_get_bound().
 */
#ifndef __MOCK_CAP_GET_BOUND_H__
#define __MOCK_CAP_GET_BOUND_H__

#include <sys/capability.h>

/**
 * Mock function for cap_get_bound().
 */
int __wrap_cap_get_bound(cap_value_t capVal);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CAP_GET_BOUND_H__ */
