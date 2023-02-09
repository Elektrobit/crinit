/**
 * @file mock-scandir.h
 * @brief Header declaring a mock function for scandir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_SCANDIR_H__
#define __MOCK_SCANDIR_H__

#include <dirent.h>
#include <sys/types.h>

/**
 * Mock function for scandir().
 *
 * Checks that the right parameters are given and return a preset value.
 */
int __wrap_scandir(const char *dirp,  // NOLINT(readability-identifier-naming)
                   struct dirent ***namelist, int (*filter)(const struct dirent *),
                   int (*compar)(const struct dirent **, const struct dirent **));
// Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_SCANDIR_H__ */
