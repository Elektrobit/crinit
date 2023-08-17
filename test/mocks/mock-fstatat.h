// SPDX-License-Identifier: MIT
/**
 * @file mock-fstatat.h
 * @brief Header declaring a mock function for fstatat().
 */
#ifndef __MOCK_FSTATAT_H__
#define __MOCK_FSTATAT_H__

#include <fcntl.h>
#include <sys/stat.h>

/**
 * Mock function for fstatat().
 *
 * Checks that the right parameters are given and return a preset value.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_fstatat(int fd, const char *path, struct stat *buf, int flag);

#endif /* __MOCK_FSTATAT_H__ */
