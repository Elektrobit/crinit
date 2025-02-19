// SPDX-License-Identifier: MIT
/**
 * @file mock-mount.c
 * @brief Implementation of a mock function for mount().
 */
#include "mock-mount.h"

#include "common.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_mount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags,
                 const void *data) {
    CRINIT_PARAM_UNUSED(source);
    CRINIT_PARAM_UNUSED(target);
    CRINIT_PARAM_UNUSED(filesystemtype);
    CRINIT_PARAM_UNUSED(mountflags);
    CRINIT_PARAM_UNUSED(data);

    return 0;
}
