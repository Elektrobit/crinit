// SPDX-License-Identifier: MIT
/**
 * @file mock-getpwuid_r.h
 * @brief Header declaring a mock function for getpwuid_r().
 */
#ifndef __MOCK_GETPWUID_R_H__
#define __MOCK_GETPWUID_R_H__

#include <fcntl.h>
#include <pwd.h>
#include <stddef.h>
#include <sys/stat.h>

/**
 * Mock function for getpwuid_r().
 *
 * Checks that the right parameters are given and return a preset value.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_getpwuid_r(uid_t uid,
                      struct passwd *__restrict resultbuf,
                      char *__restrict buffer, size_t buflen,
                      struct passwd **__restrict result);

#endif /* __MOCK_GETPWUID_R_H__ */
