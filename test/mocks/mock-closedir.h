/**
 * @file mock-closedir.h
 * @brief Header declaring a mock function for closedir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_CLOSEDIR_H__
#define __MOCK_CLOSEDIR_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for closedir().
 *
 * Checks that the right parameters are given.
 */
void __wrap_closedir(DIR *dirp);  // NOLINT(readability-identifier-naming)
                                  // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_CLOSEDIR_H__ */
