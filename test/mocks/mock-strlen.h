/**
 * @file mock-strlen.h
 * @brief Header declaring a mock function for strlen().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_STRLEN_H__
#define __MOCK_STRLEN_H__

#include <string.h>

/**
 * Mock function for strlen().
 *
 * Checks that the right parameter is given and returns a preset value.
 */
size_t __wrap_strlen(const char *s);  // NOLINT(readability-identifier-naming)
                                     // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_STRLEN_H__ */
