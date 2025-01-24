// SPDX-License-Identifier: MIT
/**
 * @file mock-kill.h
 * @brief Header declaring a mock function for kill().
 */
#ifndef __MOCK_KILL_H__
#define __MOCK_KILL_H__

#include <signal.h>

/**
 * Mock function for kill().
 *
 * Does nothing.
 */
int __wrap_kill(pid_t pid, int sig);    // NOLINT(readability-identifier-naming)
                                        // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_KILL_H__ */
