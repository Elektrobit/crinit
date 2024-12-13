// SPDX-License-Identifier: MIT
/**
 * @file mock-getgrnam_r.h
 * @brief Header declaring a mock function for getgrnam_r().
 */
#ifndef __MOCK_GETGRNAM_R_H__
#define __MOCK_GETGRNAM_R_H__

#include <fcntl.h>
#include <grp.h>
#include <stddef.h>
#include <sys/stat.h>

/**
 * Mock function for getgrnam_r().
 *
 * Checks that the right parameters are given and return a preset value.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_getgrnam_r(const char *__restrict __name,
                      struct group *__restrict __resultbuf,
                      char *__restrict __buffer, size_t __buflen,
                      struct group **__restrict __result);

#endif /* __MOCK_GETPWNAM_R_H__ */
