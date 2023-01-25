/**
 * @file mock-scandir.c
 * @brief Implementation of a mock function for scandir().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-scandir.h"

#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_scandir(const char *dirp, struct dirent ***namelist, int (*filter)(const struct dirent *),
                   int (*compar)(const struct dirent **, const struct dirent **)) {
    check_expected_ptr(dirp);
    check_expected_ptr(namelist);
    check_expected_ptr(filter);
    check_expected_ptr(compar);
    *namelist = mock_ptr_type(struct dirent **);
    return mock_type(int);
}
