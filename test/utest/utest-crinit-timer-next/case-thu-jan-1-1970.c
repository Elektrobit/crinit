// SPDX-License-Identifier: MIT

#include "common.h"
#include "timer.h"
#include "unit_test.h"
#include "utest-crinit-timer-next.h"

void crinitTimerNextTimeThuJan11970(void **state) {
    CRINIT_PARAM_UNUSED(state);

    struct timespec now = {.tv_sec = 0, .tv_nsec = 12};

    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.wDay = 1 << 3;
    td.years[0] = 1970;
    td.years[1] = 1970;
    td.month[0] = 1;
    td.month[1] = 2;
    td.days[0] = 1;
    td.days[1] = 1;
    td.hours[0] = 0;
    td.hours[1] = 23;
    td.minutes[0] = 0;
    td.minutes[1] = 59;
    td.seconds[0] = 10;
    td.seconds[1] = 10;

    char buf[100];
    crinitSPrintTimerDef(buf, &td);
    print_message("Timer: %s\n", buf);

    struct timespec next = crinitTimerNextTime(&now, &td);
    assert_int_equal(next.tv_sec, 10);
    assert_int_equal(next.tv_nsec, 12);
    next = crinitTimerNextTime(&next, &td);
    assert_int_equal(next.tv_sec, 70);
    assert_int_equal(next.tv_nsec, 12);
    next = crinitTimerNextTime(&next, &td);
    assert_int_equal(next.tv_sec, 130);
    assert_int_equal(next.tv_nsec, 12);
    next = crinitTimerNextTime(&next, &td);
    assert_int_equal(next.tv_sec, 190);
    assert_int_equal(next.tv_nsec, 12);
    next = crinitTimerNextTime(&next, &td);
    assert_int_equal(next.tv_sec, 250);
    assert_int_equal(next.tv_nsec, 12);
    next = crinitTimerNextTime(&next, &td);
    assert_int_equal(next.tv_sec, 310);
    assert_int_equal(next.tv_nsec, 12);
}
