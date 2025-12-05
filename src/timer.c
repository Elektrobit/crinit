// SPDX-License-Identifier: MIT
/**
 * @file timer.c
 * @brief Implementation of functions related to a single timer.
 */
#include "timer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "logio.h"

/**
 * check if a year is a leap year
 *
 * @param year  the year to check
 *
 * @return true if it's a leap year, false if not
 */
static bool crinitIsLeapYear(uint16_t year);
/**
 * get a static string with the short month name
 *
 * @param m  the month starting at 1 for Jan
 *
 * @return "Jan" for 1, "Feb" for 2, ...
 */
static const char *crinitMonthToStr(uint8_t m);
/**
 * Get the number of days a month is long.
 *
 * @param month  the month to get the length of
 * @param year   the year to know if Feb is 28 or 29 days long
 *
 * @return the number of days the month is long
 */
static int crinitMonthLength(uint8_t month, uint16_t year);
/**
 * Get the next value for a range.
 *
 * @param last      the value to get the next from
 * @param interval  the interval in which the next needs to be
 * @param start     the lowest possible value
 * @param limit     the highest possible value
 *
 * @param res: next value after last
 * @param off: the offset to the last value
 *
 * @return true if the res is after last and false if the next interval needs to be updated as well
 */
static bool crinitNext(int last, uint8_t interval[2], int start, int limit, int *res, long long int *offs);
/**
 * get the next year
 *
 * @param last      the year from which to start finding the next
 * @param interval  the interval in which the next year needs to be in
 *
 * @param res   the resulting next year
 * @param offs  the offset the next year has to the last
 *
 * @return true if a next year is found/possible in the interval, false if not
 */
static bool crinitNextYear(int last, uint16_t interval[2], int *res, long long int *offs);
/**
 * Returns the number of days from start of the year to the first of the specified month
 * ie. 0 for 1 (Jan), 31 for 2 (Feb), 59 (or 60 in a leap year) for 3 (Mar) and so on
 *
 * @param m     the month to get how many days the year had before the fist of it
 * @param year  the year to check in
 *
 * @return  the number of days the year had before the first of that month
 */
static int crinitDaysToMonth(uint8_t m, uint16_t year);

void crinitTimerSetDefault(crinitTimerDef_t *td) {
    td->wDay = 0x7f;
    td->years[0] = 0;
    td->years[1] = 0xffff;
    td->month[0] = 1;
    td->month[1] = 12;
    td->days[0] = 1;
    td->days[1] = 31;
    td->hours[0] = 0;
    td->hours[1] = 0;
    td->minutes[0] = 0;
    td->minutes[1] = 0;
    td->seconds[0] = 0;
    td->seconds[1] = 0;
    td->timezone[0] = 0;
    td->timezone[1] = 0;
}

static bool crinitIsLeapYear(uint16_t year) {
    return (year % 4 == 0) && (!(year % 100 == 0) || (year % 400 == 0));
}

static const char *const crinitMonthLookup[] = {
    [0] = "",    [1] = "Jan", [2] = "Feb", [3] = "Mar",  [4] = "Apr",  [5] = "May",  [6] = "Jun",
    [7] = "Jul", [8] = "Aug", [9] = "Sep", [10] = "Oct", [11] = "Nov", [12] = "Dec",
};

static const char *crinitMonthToStr(uint8_t m) {
    return m <= 12 ? crinitMonthLookup[m] : "";
}

int crinitSPrintTimerDef(char *s, crinitTimerDef_t *td) {
    int off = 0;
    if (td->years[0] == td->years[1]) {
        off += sprintf(s + off, "%04d-", td->years[0]);
    } else {
        off += sprintf(s + off, "%04d..%04d-", td->years[0], td->years[1]);
    }
    if (td->month[0] == td->month[1]) {
        off += sprintf(s + off, "%s-", crinitMonthToStr(td->month[0]));
    } else {
        off += sprintf(s + off, "%s..%s-", crinitMonthToStr(td->month[0]), crinitMonthToStr(td->month[1]));
    }
    if (td->days[0] == td->days[1]) {
        off += sprintf(s + off, "%02d ", td->days[0]);
    } else {
        off += sprintf(s + off, "%02d..%02d ", td->days[0], td->days[1]);
    }
    if (td->hours[0] == td->hours[1]) {
        off += sprintf(s + off, "%02d:", td->hours[0]);
    } else {
        off += sprintf(s + off, "%02d..%02d:", td->hours[0], td->hours[1]);
    }
    if (td->minutes[0] == td->minutes[1]) {
        off += sprintf(s + off, "%02d:", td->minutes[0]);
    } else {
        off += sprintf(s + off, "%02d..%02d:", td->minutes[0], td->minutes[1]);
    }
    if (td->seconds[0] == td->seconds[1]) {
        off += sprintf(s + off, "%02d ", td->seconds[0]);
    } else {
        off += sprintf(s + off, "%02d..%02d ", td->seconds[0], td->seconds[1]);
    }
    off += sprintf(s + off, "%c%02d%02d ", td->timezone[0] < 0 ? '-' : '+', abs(td->timezone[0]), td->timezone[1]);
    off +=
        sprintf(s + off, "|%s %s %s %s %s %s %s|", td->wDay & 1 << 0 ? "Mon" : "_  ", td->wDay & 1 << 1 ? "Tue" : "_  ",
                td->wDay & 1 << 2 ? "Wed" : "_  ", td->wDay & 1 << 3 ? "Thu" : "_  ", td->wDay & 1 << 4 ? "Fri" : "_  ",
                td->wDay & 1 << 5 ? "Sat" : "_  ", td->wDay & 1 << 6 ? "Sun" : "_  ");
    return off;
}

void crinitPrintTimerDef(crinitTimerDef_t *td) {
    char s[100];
    crinitSPrintTimerDef(s, td);
    crinitInfoPrint("@timer:%s", s);
}

bool crinitCheckTimerDef(crinitTimerDef_t *td) {
    bool res = true;
    for (size_t i = 0; i < 2; ++i) {
        if (td->month[i] == 0 || td->month[i] > 12) {
            crinitErrPrint("%02d is not a month!\n", td->month[i]);
            res = false;
        }
        if (td->days[i] > 31) {
            crinitErrPrint("%02d is not a valid day of month!\n", td->days[i]);
            res = false;
        }
        if (td->hours[i] > 23) {
            crinitErrPrint("%02d is not a valid hour of the day!\n", td->hours[i]);
            res = false;
        }
        if (td->minutes[i] > 59) {
            crinitErrPrint("%02d is not a valid minute in an hour!\n", td->hours[i]);
            res = false;
        }
        if (td->seconds[i] > 59) {
            crinitErrPrint("%02d is not a valid second in a minute!\n", td->hours[i]);
            res = false;
        }
    }
    if (0 > td->timezone[1] || 59 < td->timezone[1]) {
        crinitErrPrint("%c%02d%02d is not a well formated timezone", td->timezone[0] < 0 ? '-' : '+',
                       abs(td->timezone[0]), td->timezone[1]);
        res = false;
    } else if (-11 > td->timezone[0] || 15 < td->timezone[0]) {
        crinitErrPrint("%c%02d%02d is probbably not a correct timezone", td->timezone[0] < 0 ? '-' : '+',
                       abs(td->timezone[0]), td->timezone[1]);
    }
    if (td->month[0] == td->month[1] &&
        (4 == td->month[0] || 6 == td->month[0] || 9 == td->month[0] || 11 == td->month[0])) {
        if (31 == td->days[0] || 31 == td->days[1]) {
            crinitErrPrint("31 is not a valid day for %s!\n", crinitMonthToStr(td->month[0]));
            res = false;
        }
    }
    if (td->month[0] == td->month[1] && 2 == td->month[0]) {
        uint8_t maxWrong = 31;
        uint8_t minWrong = 30;
        if (td->years[0] == td->years[1] && crinitIsLeapYear(td->years[0])) {
            minWrong = 29;
        }
        for (size_t i = 0; i < 2; ++i) {
            if (minWrong <= td->days[i] && maxWrong > td->days[i]) {
                crinitErrPrint("%02d is not a valid day in %s!\n", td->days[i], crinitMonthToStr(2));
                res = false;
            }
        }
    }
    return res;
}

bool crinitCheckTimerTime(struct timespec ts, crinitTimerDef_t *td) {
    struct tm t;
    crinitZonedTimeR(&ts.tv_sec, td->timezone, &t);
    uint8_t mon = t.tm_mon + 1;
    uint16_t year = t.tm_year + 1900;
    uint8_t wDay = 1 << ((t.tm_wday + 6) % 7);
    return (CC_RANGE(td->seconds[0], t.tm_sec, td->seconds[1]) || CO_RANGE(t.tm_sec, td->seconds[1], td->seconds[0]) ||
            OC_RANGE(td->seconds[1], td->seconds[0], t.tm_sec)) &&
           (CC_RANGE(td->minutes[0], t.tm_min, td->minutes[1]) || CO_RANGE(t.tm_min, td->minutes[1], td->minutes[0]) ||
            OC_RANGE(td->minutes[0], td->minutes[1], t.tm_min)) &&
           (CC_RANGE(td->hours[0], t.tm_hour, td->hours[1]) || CO_RANGE(t.tm_hour, td->hours[1], td->hours[0]) ||
            OC_RANGE(td->hours[1], td->hours[0], t.tm_hour)) &&
           (CC_RANGE(td->days[0], t.tm_mday, td->days[1]) || CO_RANGE(t.tm_mday, td->days[1], td->days[0]) ||
            OC_RANGE(td->days[1], td->days[0], t.tm_mday)) &&
           (CC_RANGE(td->month[0], mon, td->month[1]) || CO_RANGE(mon, td->month[1], td->month[0]) ||
            OC_RANGE(td->month[1], td->month[0], mon)) &&
           (CC_RANGE(td->years[0], year, td->years[1]) || CO_RANGE(year, td->years[1], td->years[0]) ||
            OC_RANGE(td->years[1], td->years[0], year)) &&
           (wDay & td->wDay);
}

static int crinitMonthLength(uint8_t month, uint16_t year) {
    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            return crinitIsLeapYear(year) ? 29 : 28;
        default:
            return -1;
    }
}

static bool crinitNext(int last, uint8_t interval[2], int start, int limit, int *res, long long int *offs) {
    uint8_t range[2] = {start <= interval[0] ? interval[0] : start, interval[1] <= limit ? interval[1] : limit};
    long long int off = 0;
    bool done = false;
    if (range[0] == range[1]) {
        // if its a single possible value set the offset to the difference
        // what amount needs to change to get to that value
        off += (range[0] - last);
    } else if (range[0] < range[1]) {
        // a "standard" range of start < end
        if (CO_RANGE(range[0], last, range[1])) {
            // if last is in the range [start, end) it just 1 needs to be added
            off += 1;
        } else {
            // if not next needs to be set to start
            off += (range[0] - last);
        }
    } else if (range[0] > range[1]) {
        // if the range wraps around
        // ie is a range from start to range[1] and one from range[0] to limit
        if (CO_RANGE(range[1], last, range[0])) {
            // last is in the not allowed interval between range[1] and range[0]
            // set next to range[0]
            off += (range[0] - last);
        } else if (last == limit) {
            // last is the limit set next to start
            off -= (limit - start);
        } else {
            // otherwise just 1 can be added
            off += 1;
        }
    }
    if (off > 0) {
        // if the offset is positive the next value can be found
        // within the range without rolling over
        done = true;
    }
    *offs = off;
    *res = last + off;
    return done;
}

static bool crinitNextYear(int last, uint16_t interval[2], int *res, long long int *offs) {
    if (interval[0] == interval[1]) {
        crinitErrPrint("was only supposed to run in the year %d", last);
        return false;
    } else if (interval[0] < interval[1]) {
        if (CO_RANGE(interval[0], last, interval[1])) {
            *res = last + 1;
            *offs = 1;
            return true;
        } else {
            crinitErrPrint("%d was the last possible year", last);
            return false;
        }
    } else {
        if (last < interval[1]) {
            *res = last + 1;
            *offs = 1;
            return true;
        } else if (last >= interval[0]) {
            *offs = 1;
            *res = last + 1;
            return true;
        } else {
            *res = interval[0];
            *offs = interval[0] - last;
            crinitErrPrint("something super weird with the year %d", last);
            return false;
        }
    }
    return false;
}

static const int crinitDayToMonthLookup[] = {
    [1] = 0,   [2] = 31,  [3] = 59,  [4] = 90,   [5] = 120,  [6] = 151,
    [7] = 181, [8] = 212, [9] = 243, [10] = 273, [11] = 304, [12] = 334,
};

static int crinitDaysToMonth(uint8_t m, uint16_t year) {
    if (m != 0 && m <= 2) {
        return crinitDayToMonthLookup[m];
    } else if (m <= 12) {
        return crinitDayToMonthLookup[m] + crinitIsLeapYear(year);
    } else {
        return -1;
    }
}

struct tm *crinitZonedTimeR(const time_t *time, int8_t timezone[2], struct tm *restrict result) {
    long off = (long)timezone[0] * 3600 + (long)timezone[1] * 60;
    time_t tmpT = (*time) + off;
    gmtime_r(&tmpT, result);
    result->tm_gmtoff = off;
    return result;
}

struct timespec crinitTimerNextTime(struct timespec *last, crinitTimerDef_t *td) {
    time_t t = last->tv_sec;
    struct tm times = {0};
    crinitZonedTimeR(&t, td->timezone, &times);

    struct timespec next = {0};

    // tells if a next time is already found
    bool done = false;

    long long int offSec = 0;
    long long int offMin = 0;
    long long int offHour = 0;
    long long int offDays = 0;

    long long int off = 0;
    int partRes = 0;
    // update seconds
    done = crinitNext(times.tm_sec, td->seconds, 0, 59, &partRes, &offSec);
    if (!done || !CC_RANGE(td->minutes[0], times.tm_min, td->minutes[1])) {
        // update minutes only if last time wasn't a valid timestamp or no valid time was found yet
        // if a valid next second is still at an invalid minute the whole timestamp is still invalid
        done = crinitNext(times.tm_min, td->minutes, 0, 59, &partRes, &offMin);
    }
    if (!done || !CC_RANGE(td->hours[0], times.tm_hour, td->hours[1])) {
        done = crinitNext(times.tm_hour, td->hours, 0, 23, &partRes, &offHour);
    }
    int day = times.tm_mday;
    int month = times.tm_mon + 1;
    int year = times.tm_year + 1900;
    int tries = 0;
    do {
        if (!done || !CC_RANGE(td->days[0], day, td->days[1])) {
            int last = day;
            // special case needed for Feb 29 to stay on Feb 29 and just update the year
            if (td->days[0] == 29 && td->days[1] == 29 && td->month[0] == 2 && td->month[1] == 2 && day == 29) {
                done = false;
            } else {
                int ml = crinitMonthLength(month, year);
                done = crinitNext(last, td->days, 1, ml, &day, &off);
                offDays += off;
            }
        }
        if (!done || !CC_RANGE(td->month[0], month, td->month[1])) {
            int last = month;
            done = crinitNext(last, td->month, 1, 12, &month, &off);
            offDays += crinitDaysToMonth(month, year) - crinitDaysToMonth(last, year);
        }
        if (!done || !CC_RANGE(td->years[0], year, td->years[1])) {
            int last = year;
            done = crinitNextYear(last, td->years, &year, &off);
            offDays += off * (365 + (crinitIsLeapYear(last) ? 1 : 0));
        }
        if (!done) {
            crinitErrPrint("No possible next time found for timer");
            break;
        }
        if (done) {
            next.tv_nsec = last->tv_nsec;
            next.tv_sec = last->tv_sec + offSec + offMin * 60 + offHour * 3600 + offDays * 86400;
            done = crinitCheckTimerTime(next, td);
        }
        // because we don't consider the weekday when updating the day/month/year
        // we repeat that step until everything including the weekday is valid
        // or a limit of tries is reached that was enough reliably find Feb 29 for individual weekdays
    } while (!done && tries++ <= 40);
    if (!done) {
        next.tv_sec = 0;
        next.tv_nsec = 0;
    }
    return next;
}
