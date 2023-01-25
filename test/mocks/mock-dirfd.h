/**
 * @file mock-dirfd.h
 * @brief Header declaring a mock function for dirfd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_DIRFD_H__
#define __MOCK_DIRFD_H__

#include <sys/types.h>
#include <dirent.h>

/**
 * Mock function for dirfd().
 *
 * Checks that the right parameters are given and return a preset value.
 */
int __wrap_dirfd(DIR *dirp);  // NOLINT(readability-identifier-naming)
                              // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_DIRFD_H__ */
