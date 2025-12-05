// SPDX-License-Identifier: MIT

#include <assert.h>

#include "common.h"
#include "timer.h"
#include "unit_test.h"
#include "utest-crinit-timer-next.h"

static void crinitCheckTimerRange(struct timespec now, crinitTimerDef_t *td, long int minVal, long int maxVal, int n) {
    struct timespec next = crinitTimerNextTime(&now, td);
    assert_int_equal(next.tv_nsec, now.tv_nsec);
    assert_true(next.tv_sec > now.tv_sec);
    assert_true(next.tv_sec - now.tv_sec <= maxVal);
    for (int i = 0; i < n; i++) {
        now.tv_sec = next.tv_sec;
        struct timespec next = crinitTimerNextTime(&now, td);
        assert_int_equal(next.tv_nsec, now.tv_nsec);
        assert_true(next.tv_sec > now.tv_sec);
        assert_true(next.tv_sec - now.tv_sec >= minVal);
        assert_true(next.tv_sec - now.tv_sec <= maxVal);
    }
}

void crinitCountSecondUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.hours[0] = 0;
    td.hours[1] = 23;
    td.minutes[0] = 0;
    td.minutes[1] = 59;
    td.seconds[0] = 0;
    td.seconds[1] = 59;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Second: %s\n", buf);
    crinitCheckTimerRange(now, &td, 1, 1, 130);
}

void crinitCountMinuteUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.hours[0] = 0;
    td.hours[1] = 23;
    td.minutes[0] = 0;
    td.minutes[1] = 59;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Minute: %s\n", buf);
    crinitCheckTimerRange(now, &td, 60, 60, 130);
}

void crinitCountHourUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.hours[0] = 0;
    td.hours[1] = 23;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Hour:   %s\n", buf);
    crinitCheckTimerRange(now, &td, 3600, 3600, 49);
}

void crinitCountDayUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Day:    %s\n", buf);
    crinitCheckTimerRange(now, &td, 86400, 86400, 65);
}

void crinitCountWeekUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.wDay = 1;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Week:   %s\n", buf);
    crinitCheckTimerRange(now, &td, 604800, 604800, 65);
}

void crinitCountMonthUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.days[0] = 1;
    td.days[1] = 1;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Month:  %s\n", buf);
    crinitCheckTimerRange(now, &td, 2419200, 2678400, 30);
}

void crinitCountYearUp(struct timespec now) {
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.days[0] = 1;
    td.days[1] = 1;
    td.month[0] = 1;
    td.month[1] = 1;
    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Year:   %s\n", buf);
    crinitCheckTimerRange(now, &td, 31536000, 31622400, 20);
}

void crinitCountChecks(struct timespec now) {
    crinitCountSecondUp(now);
    crinitCountMinuteUp(now);
    crinitCountHourUp(now);
    crinitCountDayUp(now);
    crinitCountWeekUp(now);
    crinitCountMonthUp(now);
    crinitCountYearUp(now);
}

void crinitTimerNextTimeRangeSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);
    struct timespec now = {.tv_sec = 1763460667, .tv_nsec = 12};
    crinitCountChecks(now);
    now.tv_sec = time(NULL);
    now.tv_nsec = 30242;
    crinitCountChecks(now);
    now.tv_sec = 0;
    now.tv_nsec = 0;
    crinitCountChecks(now);
}
