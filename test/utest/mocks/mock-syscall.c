// SPDX-License-Identifier: MIT
/**
 * @file mock-syscall.c
 * @brief Implementation of a mock function for syscall().
 */
#include <sys/capability.h>
#include <sys/syscall.h>

#include "common.h"
#include "logio.h"
#include "unit_test.h"

// Rationale: Naming scheme fixed due to linker wrapping.
// NOLINTNEXTLINE(readability-identifier-naming)
int __wrap_syscall(long number, ...) {
    check_expected(number);

    if (number == SYS_capget) {
        va_list args;
        va_start(args, number);

        struct __user_cap_header_struct *hdr = va_arg(args, struct __user_cap_header_struct *);
        check_expected_ptr(hdr);
        check_expected(hdr->version);
        check_expected(hdr->pid);

        cap_user_data_t out = va_arg(args, cap_user_data_t);
        check_expected_ptr(out);

        out[0].inheritable = mock_type(uint32_t);
        out[1].inheritable = mock_type(uint32_t);

        out[0].effective = mock_type(uint32_t);
        out[1].effective = mock_type(uint32_t);
    }

    return 0;
}
