// SPDX-License-Identifier: MIT
/**
 * @file mock-getgrnam_r.c
 * @brief Implementation of a mock function for getgrnam_r().
 */
#include "mock-getgrnam_r.h"

#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_getgrnam_r(const char *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer,
                      size_t buflen, struct group **__restrict result) {
    assert_non_null(name);
    assert_non_null(resultbuf);
    assert_non_null(buffer);
    CRINIT_PARAM_UNUSED(buflen);
    assert_non_null(result);

    if (strcmp(name, "disk") == 0) {
        resultbuf->gr_gid = 42;
        strncpy(buffer, name, buflen);
        resultbuf->gr_name = buffer;
        *result = resultbuf;
    } else if (strcmp(name, "floppy") == 0) {
        resultbuf->gr_gid = 15;
        strncpy(buffer, name, buflen);
        resultbuf->gr_name = buffer;
        *result = resultbuf;
    } else if (strcmp(name, "nogroup") == 0) {
        resultbuf->gr_gid = 65534;
        strncpy(buffer, name, buflen);
        resultbuf->gr_name = buffer;
        *result = resultbuf;
    } else {
        *result = NULL;
    }

    return mock_type(int);
}
