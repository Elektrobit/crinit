// SPDX-License-Identifier: MIT
/**
 * @file mock-reboot.h
 * @brief Header declaring a mock function for reboot().
 */
#ifndef __MOCK_REBOOT_H__
#define __MOCK_REBOOT_H__

/**
 * Mock function for reboot().
 *
 * Does nothing.
 */
int __wrap_reboot(int cmd);     // NOLINT(readability-identifier-naming)
                                // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_REBOOT_H__ */
