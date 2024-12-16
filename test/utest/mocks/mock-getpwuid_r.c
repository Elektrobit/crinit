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
                      struct passwd *__restrict resultbuf,
                      char *__restrict buffer, size_t buflen,
                      struct passwd **__restrict result) {
    assert_non_null(resultbuf);
    assert_non_null(buffer);
    CRINIT_PARAM_UNUSED(buflen);
    assert_non_null(result);

    if (uid == 42) {
        resultbuf->pw_uid = 42;
        strncpy(buffer, "www-run", buflen);
        resultbuf->pw_name = buffer;
        *result = resultbuf;
    }
    else {
        *result = NULL;
    }

    return mock_type(int);
}
