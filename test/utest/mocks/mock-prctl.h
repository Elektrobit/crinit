// SPDX-License-Identifier: MIT
/**
 * @file mock-prctl.h
 * @brief Header declaring a mock function for prctl().
 */
#ifndef __MOCK_PRCTL_H__
#define __MOCK_PRCTL_H__

/**
 * Mock function for prctl().
 */
int __wrap_prctl(int op, ...);  // NOLINT(readability-identifier-naming)
                                // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_PRCTL_H__ */
