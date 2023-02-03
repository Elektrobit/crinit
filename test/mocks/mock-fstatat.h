/**
 * @file mock-fstatat.h
 * @brief Header declaring a mock function for fstatat().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
int __wrap_fstatat(int fd, const char *path, struct stat *buf,
                   int flag);  // NOLINT(readability-identifier-naming)
                               // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_FSTATAT_H__ */
