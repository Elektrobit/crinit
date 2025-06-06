// SPDX-License-Identifier: MIT
/**
 * @file mock-syscall.h
 * @brief Header declaring a mock function for syscall().
 */
#ifndef __MOCK_SYSCALL_H__
#define __MOCK_SYSCALL_H__

#include <sys/capability.h>

/**
 * Mock function for syscall().
 */
int __wrap_syscall(long number, ...);  // NOLINT(readability-identifier-naming)
                                       // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_SYSCALL_H__ */
