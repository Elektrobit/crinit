// SPDX-License-Identifier: MIT
/**
 * @file mock-getpwnam_r.h
 * @brief Header declaring a mock function for getpwnam_r().
 */
#ifndef __MOCK_GETPWNAM_R_H__
#define __MOCK_GETPWNAM_R_H__

#include <fcntl.h>
#include <pwd.h>
#include <stddef.h>
#include <sys/stat.h>

/**
 * Mock function for getpwnam_r().
 *
 * Checks that the right parameters are given and return a preset value.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_getpwnam_r(const char *__restrict name,
                      struct passwd *__restrict resultbuf,
                      char *__restrict buffer, size_t buflen,
                      struct passwd **__restrict result);

#endif /* __MOCK_GETPWNAM_R_H__ */
