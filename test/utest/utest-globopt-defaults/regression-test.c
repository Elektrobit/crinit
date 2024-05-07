// SPDX-License-Identifier: MIT
/**
 * @file regression-test.c
 * @brief Implementation of a regression test if default values for all global options are present and intialized.
 */
#include <string.h>

#include "common.h"
#include "confmap.h"
#include "globopt.h"
#include "logio.h"
#include "unit_test.h"
#include "utest-globopt-defaults.h"

/** The test pattern to pre-init the global option struct with. **/
#define CRINIT_BIT_TEST_PATTERN 0xAA
/** A pointer to be returned by allocation functions, different from #CRINIT_BIT_TEST_PATTERN and NULL. **/
#ifdef __LP64__
#define CRINIT_FAKE_POINTER 0xC001C0D366336633LLu  // 64-Bit pointer size
#else
#define CRINIT_FAKE_POINTER 0xC001C0D3Lu  // 32-Bit pointer size
#endif

void crinitGlobDefRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // Make memset a no-op in global option intialization function
    expect_any_always(__wrap_memset, str);
    expect_any_always(__wrap_memset, c);
    expect_any_always(__wrap_memset, n);
    will_return(__wrap_memset, CRINIT_FAKE_POINTER);

    // calloc and strdup shall always successfully return the same known value inside the global option initialization
    // function.
    expect_any_always(__wrap_calloc, num);
    expect_any_always(__wrap_calloc, size);
    will_return_maybe(__wrap_calloc, CRINIT_FAKE_POINTER);
    expect_any_always(__wrap_strdup, s);
    will_return_maybe(__wrap_strdup, CRINIT_FAKE_POINTER);

    // Pre-init global option struct with test pattern.
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    assert_ptr_not_equal(globOpts, NULL);
    for (size_t i = 0; i < sizeof(*globOpts); i++) {
        ((uint8_t *)globOpts)[i] = CRINIT_BIT_TEST_PATTERN;
    }
    assert_int_equal(crinitGlobOptRemit(), 0);

    // Run global option initialization function
    assert_int_equal(crinitGlobOptInitDefault(), 0);

    // Search for remaining test pattern in global option struct. Fail if any bytes found.
    globOpts = crinitGlobOptBorrow();
    for (size_t i = 0; i < sizeof(*globOpts); i++) {
        if (((uint8_t *)globOpts)[i] == CRINIT_BIT_TEST_PATTERN) {
            fprintf(stderr,
                    "Uninitialized part of global option struct found at offset %zu Bytes (note that this test is "
                    "built with -fpack-struct when determining the member variable).\n",
                    i);
            assert_int_not_equal(((uint8_t *)globOpts)[i], CRINIT_BIT_TEST_PATTERN);
        }
    }
    assert_int_equal(crinitGlobOptRemit(), 0);
}
