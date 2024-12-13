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
int __wrap_getgrgid_r(gid_t gid, struct group *__restrict __resultbuf,
                      char *__restrict __buffer, size_t __buflen,
                      struct group **__restrict __result) {
    assert_non_null(__resultbuf);
    assert_non_null(__buffer);
    CRINIT_PARAM_UNUSED(__buflen);
    assert_non_null(__result);

    if (gid == 42) {
        __resultbuf->gr_gid = 42;
        strncpy(__buffer, "disk", __buflen);
        __resultbuf->gr_name = __buffer;
        *__result = __resultbuf;
    }
    else {
        *__result = NULL;
    }

    return mock_type(int);
}
