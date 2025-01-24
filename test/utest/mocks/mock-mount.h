// SPDX-License-Identifier: MIT
/**
 * @file mock-mount.h
 * @brief Header declaring a mock function for mount().
 */
#ifndef __MOCK_MOUNT_H__
#define __MOCK_MOUNT_H__

/**
 * Mock function for mount().
 *
 * Does nothing.
 */
int __wrap_mount(const char *source, const char *target,                // NOLINT(readability-identifier-naming)
                 const char *filesystemtype, unsigned long mountflags,  // NOLINT(readability-identifier-naming)
                 const void * data);                                    // NOLINT(readability-identifier-naming)
                                                                        // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_MOUNT_H__ */
