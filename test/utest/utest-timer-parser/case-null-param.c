// SPDX-License-Identifier: MIT
/**
 * @file case-success.c
 * @brief Unit test for crinitClientTaskAdd(), successful execution.
 */

#include <stdint.h>

#include "common.h"
#include "string.h"
#include "timer.h"
#include "unit_test.h"
#include "utest-timer-parser.h"

#define DEBUG_PRINT 1

void crinitTimerParserTestNullParam(void **state) {
    CRINIT_PARAM_UNUSED(state);

    bool res;
    char *s[] = {
        "",
        "somethin else",
        "Sat..Tue-23..1:00",
    };
#if DEBUG_PRINT
    print_message("fail at parsing into NULL timer\n");
#endif
    for (size_t i = 0; i < ARRAY_SIZE(s); i++) {
        res = crinitTimerParse(s[i], NULL);
        assert_false(res);
    }
#if DEBUG_PRINT
    print_message("fail at parsing NULL string\n");
#endif
    crinitTimerDef_t timerRes = {0};
    res = crinitTimerParse(NULL, &timerRes);
    assert_false(res);
#if DEBUG_PRINT
    print_message("fail at parsing with both parameters NULL\n");
#endif
    res = crinitTimerParse(NULL, NULL);
    assert_false(res);
}
