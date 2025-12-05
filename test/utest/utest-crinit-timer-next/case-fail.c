// SPDX-License-Identifier: MIT

#include "common.h"
#include "timer.h"
#include "unit_test.h"
#include "utest-crinit-timer-next.h"

void crinitTimerNextTimeFail(void **state) {
    CRINIT_PARAM_UNUSED(state);
    struct timespec now = {.tv_sec = 1763460667, .tv_nsec = 12};
    crinitTimerDef_t td = {0};
    crinitTimerSetDefault(&td);
    td.years[1] = 2000;
    struct timespec next = crinitTimerNextTime(&now, &td);
    assert_int_equal(next.tv_sec, 0);
    assert_int_equal(next.tv_nsec, 0);
}
