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

void crinitCheckTimerParse(char *s) {
    crinitTimerDef_t timerRes = {0};
#if DEBUG_PRINT
    print_message("fail at parsing: %s\n", s);
#endif
    bool res = crinitTimerParse(s, &timerRes);
    assert_false(res);
}

void crinitTimerParserTestError(void **state) {
    CRINIT_PARAM_UNUSED(state);

    char *s[] = {
        "-2..5-15..31",
        "0001..2030--15..31",
        "1..2030-2..5-15..32-00:00:00+0000",
        "0001..2030-02..05-15..31-24:60:00+0000",
        "*-0001.2030-02..05-15..31-00:00:00+0000",
        "MON..SUN-0001..2030-02..05-015..31-00:000:00+000",
        "MON,SUN,TUE-01,12..2030-02..05-15..31-00:00:00+0",
        "MON..SUN 0001..2030-02..05-15..31 00:00:00 +0000",
        "",
        "somethin else",
        "*-",
        "yesterday",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s); i++) {
        crinitCheckTimerParse(s[i]);
    }
}
