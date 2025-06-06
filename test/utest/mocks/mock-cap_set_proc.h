// SPDX-License-Identifier: MIT
/**
 * @file mock-cap_set_proc.h
 * @brief Header declaring a mock function for cap_set_proc().
 */
#ifndef __MOCK_CAP_SET_PROC_H__
#define __MOCK_CAP_SET_PROC_H__

#include <sys/capability.h>

/**
 * Mock function for prctl().
 *
 * Does nothing.
 */
int __wrap_cap_set_proc(cap_t cap);  // NOLINT(readability-identifier-naming)
                                     // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CAP_SET_PROC_H__ */
