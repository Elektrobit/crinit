#include "mock_globopt_set.h"

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
// Rationale: Specific order of includes needed by cmocka.h.

int __wrap_EBCL_globOptSet(ebcl_GlobOptKey_t key, const void *val,  // NOLINT(readability-identifier-naming)
                           size_t sz) {                             // Rationale: Naming scheme fixed due to linker
                                                                    // wrapping.
    check_expected(key);
    check_expected_ptr(val);
    check_expected(sz);
    return mock_type(int);
}
