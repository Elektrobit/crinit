/**
 * @file mock-opendir.h
 * @brief Header declaring a mock function for opendir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_OPENDIR_H__
#define __MOCK_OPENDIR_H__

#include <sys/types.h>
#include <dirent.h>

/**
 * Mock function for opendir().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
DIR *__wrap_opendir(const char *name);  // NOLINT(readability-identifier-naming)
                                   // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_OPENDIR_H__ */
