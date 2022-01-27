// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
// Rationale: Specific order of includes needed by cmocka.h.

#include "utest-crinit-set-verbose.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const struct CMUnitTest tests[] = {cmocka_unit_test(EBCL_crinitSetVerboseTestSuccess),
                                       cmocka_unit_test(EBCL_crinitSetVerboseTestGlobOptError)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
