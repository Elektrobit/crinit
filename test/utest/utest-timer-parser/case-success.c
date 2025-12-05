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

static void crinitCheckTimer(crinitTimerDef_t *a, crinitTimerDef_t *b) {
    assert_int_equal(a->seconds[0], b->seconds[0]);
    assert_int_equal(a->seconds[1], b->seconds[1]);
    assert_int_equal(a->minutes[0], b->minutes[0]);
    assert_int_equal(a->minutes[1], b->minutes[1]);
    assert_int_equal(a->hours[0], b->hours[0]);
    assert_int_equal(a->hours[1], b->hours[1]);
    assert_int_equal(a->days[0], b->days[0]);
    assert_int_equal(a->days[1], b->days[1]);
    assert_int_equal(a->month[0], b->month[0]);
    assert_int_equal(a->month[1], b->month[1]);
    assert_int_equal(a->years[0], b->years[0]);
    assert_int_equal(a->years[1], b->years[1]);
    uint8_t wDayA = a->wDay & 0x7f;
    uint8_t wDayB = b->wDay & 0x7f;
    assert_int_equal(wDayA, wDayB);
}

static void crinitCheckTimerParse(char *s, crinitTimerDef_t *timer) {
    crinitTimerDef_t timerRes = {0};
#if DEBUG_PRINT
    print_message("parsing: %s => ", s);
#endif
    bool res = crinitTimerParse(s, &timerRes);
#if DEBUG_PRINT
    crinitPrintTimerDef(&timerRes);
#endif
    assert_true(res);
    crinitCheckTimer(&timerRes, timer);
}

void crinitTimerParserTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    crinitTimerDef_t timer = {0};
    crinitTimerSetDefault(&timer);
    timer.years[0] = 1;
    timer.years[1] = 2030;
    timer.month[0] = 2;
    timer.month[1] = 5;
    timer.days[0] = 15;
    timer.days[1] = 31;
    char *s0[] = {
        "1..2030-2..5-15..31",
        "0001..2030-02..05-15..31",
        "1..2030-2..5-15..31-00:00:00+0000",
        "0001..2030-02..05-15..31-00:00:00+0000",
        "*-0001..2030-02..05-15..31-00:00:00+0000",
        "MON..SUN-0001..2030-02..05-15..31-00:00:00+0000",
        "MON..SUN-0001..2030-02..05-15..31-00:00:00+000",
        "MON..SUN-0001..2030-02..05-15..31-00:00:00+00",
        "MON..SUN-0001..2030-02..05-15..31-00:00:00+0",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s0); i++) {
        crinitCheckTimerParse(s0[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.wDay = 0x63;
    timer.hours[0] = 23;
    timer.hours[1] = 1;
    char *s1[] = {
        "Sat..Tue-23..1:00",
        "Saturday..Tuesday-*-*-*-23..01:00",
        "SATURDAY..tuesday-*-*-*-23..01:00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s1); i++) {
        crinitCheckTimerParse(s1[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.wDay = 1;
    timer.hours[0] = 4;
    timer.hours[1] = 13;
    timer.minutes[0] = 2;
    timer.minutes[1] = 30;
    timer.seconds[0] = 1;
    timer.seconds[1] = 55;
    char *s2[] = {
        "Monday-4..13:2..30:1..55",
        "MON-04..13:02..30:01..55",
        "monday-4..13:2..30:1..55+0000",
        "mon-04..13:02..30:01..55+0000",
        "MONDAY-*-*-*-04..13:02..30:01..55+0000",
        "mon-*-*-*-04..13:02..30:01..55",
        "Mon-0000..65535-01..12-01..31-04..13:02..30:01..55",
        "mon-0000..65535-01..12-01..31-04..13:02..30:01..55+0000",
        "mon-0000..65535-01..12-01..31-04..13:02..30:01..55+0000",
        "mon-0000..65535-01..12-01..31-04..13:02..30:1..55+0000",
        "mon-..65535-01..12-01..31-04..13:02..30:1..55+0000",
        "mon-0..-01..12-01..31-04..13:02..30:1..55+0000",
        "mon-..-01..12-01..31-04..13:02..30:1..55+0000",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s2); i++) {
        crinitCheckTimerParse(s2[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.years[0] = 1;
    timer.years[1] = 1;
    timer.month[1] = 1;
    timer.days[1] = 1;
    char *s3[] = {
        "1-1-1", "0001-01-01", "1-1-1-00:00:00+0000", "0001-01-01-00:00:00+0000", "MON..SUN-0001-01-01-00:00:00+0000",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s3); i++) {
        crinitCheckTimerParse(s3[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.wDay = 0x70;
    char *s4[] = {
        "Fri..Sun",
        "FRI..sun-*-*-*",
        "FRI..sun-*-*-*-0:0",
        "Friday..Sunday-*-*-*-0:0:0",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s4); i++) {
        crinitCheckTimerParse(s4[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.wDay = 0x60;
    timer.hours[0] = 12;
    timer.hours[1] = 12;
    timer.timezone[0] = 1;
    char *s5[] = {
        "Sat..Sun-12:00:00+1",
        "sat..sun-*-*-*-12:0+01",
        "SAT..sun-*-*-*-12:0+100",
        "SATURDAY..Sunday-*-*-*-12:0:0+01:00",
        "SATURDAY..Sunday-*-*-*-12:0:0+0100",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s5); i++) {
        crinitCheckTimerParse(s5[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.wDay = 1;
    char *s6[] = {
        "weekly", "Weekly", "WEEKLY", "Monday", "MONDAY-*-*-*-00:00:00+00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s6); i++) {
        crinitCheckTimerParse(s6[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    char *s7[] = {
        "daily",
        "Daily",
        "DAILY",
        "MONDAY..SUN-*-*-*-00:00:00+00:00",
        "*-*-*-*-00:00:00+00:00",
        "*-*-*-*",
        "*-00:00:00+00:00",
        "*",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s7); i++) {
        crinitCheckTimerParse(s7[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.hours[1] = 23;
    timer.minutes[1] = 59;
    char *s8[] = {
        "minutely",
        "Minutely",
        "MINUTELY",
        "*:*",
        "*-*-*-*:*:00+00:00",
        "*:*:00+00:00",
        "MONDAY..SUN-*-*-*-*:*:00+00:00",
        "MONDAY..SUN-*-*-*-00..23:00..59:00+00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s8); i++) {
        crinitCheckTimerParse(s8[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.hours[1] = 23;
    char *s9[] = {
        "hourly",
        "Hourly",
        "HOURLY",
        "*:00",
        "MONDAY..SUN-*:00:00+00:00",
        "MONDAY..SUN-*:00",
        "*-*-*-*:00:00+00:00",
        "MONDAY..SUN-*-*-*-*:00:00+00:00",
        "MONDAY..SUN-*-*-*-00..23:00:00+00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s9); i++) {
        crinitCheckTimerParse(s9[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.days[1] = 1;
    char *s10[] = {
        "monthly",
        "Monthly",
        "MONTHLY",
        "*-*-01",
        "*-*-01-00:00:00+00:00",
        "MONDAY..SUN-*-*-01",
        "MONDAY..SUN-*-*-01-00:00:00+00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s10); i++) {
        crinitCheckTimerParse(s10[i], &timer);
    }

    crinitTimerSetDefault(&timer);
    timer.days[1] = 1;
    timer.month[1] = 1;
    char *s11[] = {
        "yearly",
        "Yearly",
        "YEARLY",
        "annually",
        "Annually",
        "ANNUALLY",
        "*-01-01",
        "*-01-01-00:00:00+00:00",
        "MONDAY..SUN-*-01-01",
        "MONDAY..SUN-*-01-01-00:00:00+00:00",
        "MONDAY..SUN-0..65535-01-01-00:00:00+00:00",
    };
    for (size_t i = 0; i < ARRAY_SIZE(s11); i++) {
        crinitCheckTimerParse(s11[i], &timer);
    }
}
