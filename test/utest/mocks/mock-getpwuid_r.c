// SPDX-License-Identifier: MIT
/**
 * @file mock-getpwuid_r.c
 * @brief Implementation of a mock function for getpwnam_r().
 */
#include "mock-getpwuid_r.h"

#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "unit_test.h"



// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_getpwuid_r(uid_t uid,
                      struct passwd *__restrict __resultbuf,
                      char *__restrict __buffer, size_t __buflen,
                      struct passwd **__restrict __result) {
    assert_non_null(__resultbuf);
    assert_non_null(__buffer);
    CRINIT_PARAM_UNUSED(__buflen);
    assert_non_null(__result);

    if (uid == 42) {
        __resultbuf->pw_uid = 42;
        strncpy(__buffer, "www-run", __buflen);
        __resultbuf->pw_name = __buffer;
        *__result = __resultbuf;
    }
    else {
        *__result = NULL;
    }

    return mock_type(int);
}
