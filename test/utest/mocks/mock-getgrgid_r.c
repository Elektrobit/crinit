// SPDX-License-Identifier: MIT
/**
 * @file mock-getgrgid_r.c
 * @brief Implementation of a mock function for getgrgid_r().
 */
#include "mock-getgrgid_r.h"

#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_getgrgid_r(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen,
                      struct group **__restrict result) {
    assert_non_null(resultbuf);
    assert_non_null(buffer);
    CRINIT_PARAM_UNUSED(buflen);
    assert_non_null(result);

    if (gid == 42) {
        resultbuf->gr_gid = 42;
        strncpy(buffer, "disk", buflen);
        resultbuf->gr_name = buffer;
        *result = resultbuf;
    } else {
        *result = NULL;
    }

    return mock_type(int);
}
