// SPDX-License-Identifier: MIT
/**
 * @file timer_parser.c
 * @brief Implementation of a parser for timer definitions build with re2c.
 */
#include "timer.h"
#include "logio.h"
#include "common.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/*!conditions:re2c*/

/**
 * set the range of weekdays for a timer
 * is not guaranteed to give useful results if start or end aren't a power of two
 *
 * @param start the first day of the range (Mon = 1, Tue = 1 << 1, Wed = 1 << 2, ...)
 * @param end   the last day of the range
 * @param td    the crinitTimerDef_t to set the weekday field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetWeekdays(uint8_t start, uint8_t end, crinitTimerDef_t *td);
/**
 * parse uint16_t from a string slice
 * it is not designed for general number parsing
 * and it doesn't check if the provided string is actually just digits
 *
 * @param start  the first character to start parsing
 * @param end    after the last character to parse
 *
 * @return the parsed uint16_t on success
 *         and 0xFFFF if it's an empty string or start is after end
 */
static uint16_t crinitTimerParseUint16(const char *start, const char* end);
/**
 * set the range of years for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sY1   the first byte of the start year
 * @param eY1   after the last byte of the start year
 * @param sY2   the first byte of the end year
 * @param eY2   after the last byte of the end year
 * @param td    the crinitTimerDef_t to set the years field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetYears(const char *sY1, const char* eY1, const char *sY2, const char* eY2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sM1   the first byte of the start month
 * @param eM1   after the last byte of the start month
 * @param sM2   the first byte of the end month
 * @param eM2   after the last byte of the end month
 * @param td    the crinitTimerDef_t to set the month field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetMonth(const char *sM1, const char* eM1, const char *sM2, const char* eM2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sD1   the first byte of the start day
 * @param eD1   after the last byte of the start day
 * @param sD2   the first byte of the end day
 * @param eD2   after the last byte of the end day
 * @param td    the crinitTimerDef_t to set the day field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetDays(const char *sD1, const char* eD1, const char *sD2, const char* eD2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sH1   the first byte of the start hours
 * @param eH1   after the last byte of the start hours
 * @param sH2   the first byte of the end hours
 * @param eH2   after the last byte of the end hours
 * @param td    the crinitTimerDef_t to set the hours field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetHours(const char *sH1, const char* eH1, const char *sH2, const char* eH2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sM1   the first byte of the start minutes
 * @param eM1   after the last byte of the start minutes
 * @param sM2   the first byte of the end minutes
 * @param eM2   after the last byte of the end minutes
 * @param td    the crinitTimerDef_t to set the minutes field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetMinutes(const char *sM1, const char* eM1, const char *sM2, const char* eM2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sS1   the first byte of the start second
 * @param eS1   after the last byte of the start second
 * @param sS2   the first byte of the end second
 * @param eS2   after the last byte of the end second
 * @param td    the crinitTimerDef_t to set the second field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetSeconds(const char *sS1, const char* eS1, const char *sS2, const char* eS2, crinitTimerDef_t *td);
/**
 * set the range of month for a timer, if one of the strings is empty or start after end the min/max value is used.
 * it will not error if the strings aren't only digits but give some unexpected results.
 *
 * @param sTzH   the first byte of the timezone hours
 * @param eTzH   after the last byte of the timezone hours
 * @param sTzM   the first byte of the timezone minutes
 * @param eTzM   after the last byte of the timezone minutes
 * @param td     the crinitTimerDef_t to set the timezone field of
 *
 * @return true on success, false on error
 */
static bool crinitTimerSetTimezone(const char *sTzH, const char* eTzH, const char *sTzM, const char* eTzM, crinitTimerDef_t *td);

static bool crinitTimerSetWeekdays(uint8_t start, uint8_t end, crinitTimerDef_t *td) {
    crinitNullCheck(false, td);
    if (0xff == end) {
        td->wDay = start;
        return true;
    }
    uint8_t days = 0;
    if (start > end) {
        days = ~days;
        uint8_t tmp = start;
        start = end;
        end = tmp;
        start = start << 1;
        end = end >> 1;
    }
    do {
        days = days ^ start;
        start = start << 1;
    } while (start <= end && 0 != start);
    td->wDay = days;
    return true;
}

static uint16_t crinitTimerParseUint16(const char *start, const char* end) {
    if (start >= end) {
        return 0xffff;
    }
    uint16_t res = 0;
    for (const char *idx = start; idx < end; ++idx) {
        // because this function is only supposed to be used within the parser
        // and the regex ensures the string only contains digits
        // we don't need another check here
        res = res * 10 + (*idx - '0');
    }
    return res;
}

static bool crinitTimerSetYears(const char *sY1, const char* eY1, const char *sY2, const char* eY2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sY1, eY1, sY2, eY2);
    uint16_t start = crinitTimerParseUint16(sY1, eY1);
    uint16_t end = crinitTimerParseUint16(sY2, eY2);
    td->years[0] = start == 0xffff ? 0 : start;
    td->years[1] = end;
    return true;
}

static bool crinitTimerSetMonth(const char *sM1, const char* eM1, const char *sM2, const char* eM2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sM1, eM1, sM2, eM2);
    uint8_t start = (uint8_t)crinitTimerParseUint16(sM1, eM1);
    uint8_t end = (uint8_t)crinitTimerParseUint16(sM2, eM2);
    if (0xff == start) {
        start = 1;
    }
    if (0xff == end) {
        end = 12;
    }
    td->month[0] = start;
    td->month[1] = end;
    return CC_RANGE(1, td->month[0], 12) && CC_RANGE(1, td->month[1], 12);
}

static bool crinitTimerSetDays(const char *sD1, const char* eD1, const char *sD2, const char* eD2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sD1, eD1, sD2, eD2);
    uint8_t start = (uint8_t)crinitTimerParseUint16(sD1, eD1);
    uint8_t end = (uint8_t)crinitTimerParseUint16(sD2, eD2);
    if (0xff == start) {
        start = 1;
    }
    if (0xff == end) {
        end = 31;
    }
    td->days[0] = start;
    td->days[1] = end;
    return CC_RANGE(1, td->days[0], 31) && CC_RANGE(1, td->days[1], 31);
}

static bool crinitTimerSetHours(const char *sH1, const char* eH1, const char *sH2, const char* eH2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sH1, eH1, sH2, eH2);
    uint8_t start = (uint8_t)crinitTimerParseUint16(sH1, eH1);
    uint8_t end = (uint8_t)crinitTimerParseUint16(sH2, eH2);
    if (0xff == start) {
        start = 0;
    }
    if (0xff == end) {
        end = 23;
    }
    td->hours[0] = start;
    td->hours[1] = end;
    return CC_RANGE(0, td->hours[0], 23) && CC_RANGE(0, td->hours[1], 23);
}

static bool crinitTimerSetMinutes(const char *sM1, const char* eM1, const char *sM2, const char* eM2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sM1, eM1, sM2, eM2);
    uint8_t start = (uint8_t)crinitTimerParseUint16(sM1, eM1);
    uint8_t end = (uint8_t)crinitTimerParseUint16(sM2, eM2);
    if (0xff == start) {
        start = 0;
    }
    if (0xff == end) {
        end = 59;
    }
    td->minutes[0] = start;
    td->minutes[1] = end;
    return CC_RANGE(0, td->minutes[0], 59) && CC_RANGE(0, td->minutes[1], 59);
}

static bool crinitTimerSetSeconds(const char *sS1, const char* eS1, const char *sS2, const char* eS2, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sS1, eS1, sS2, eS2);
    uint8_t start = (uint8_t)crinitTimerParseUint16(sS1, eS1);
    uint8_t end = (uint8_t)crinitTimerParseUint16(sS2, eS2);
    if (0xff == start) {
        start = 0;
    }
    if (0xff == end) {
        end = 59;
    }
    td->seconds[0] = start;
    td->seconds[1] = end;
    return CC_RANGE(0, td->seconds[0], 59) && CC_RANGE(0, td->seconds[1], 59);
}

static bool crinitTimerSetTimezone(const char *sTzH, const char* eTzH, const char *sTzM, const char* eTzM, crinitTimerDef_t *td) {
    crinitNullCheck(false, td, sTzH, eTzH, sTzM, eTzM);
    const char *startHour = sTzH;
    int8_t sign = 1;
    if (*startHour == '-') {
        sign = -1;
    }
    startHour += 1;
    int8_t hours = (int8_t)crinitTimerParseUint16(startHour, eTzH);
    uint8_t minutes = (uint8_t)crinitTimerParseUint16(sTzM, eTzM);
    td->timezone[0] = sign * hours;
    td->timezone[1] = 0xff == minutes ? 0 : minutes;
    return CC_RANGE(-13, td->timezone[0], 15) && CC_RANGE(0, td->timezone[1], 59);
}

bool crinitTimerParse(char *s, crinitTimerDef_t *td) {
    crinitNullCheck(false, s, td);

    char *t1;
    char *t2;
    char *t3;
    char *t4;
    char *yyt1;
    char *yyt2;
    char *yyt3;
    char *yyt4;
    char *yyt5;
    char *YYCURSOR = s;
    char *YYMARKER = s;

    int c = yyctimer;

    crinitTimerSetDefault(td);

    uint8_t wDay = 0;
    bool result = true;

    /*!re2c
        re2c:api:style = free-form;
        re2c:define:YYCTYPE = char;
        re2c:define:YYGETCONDITION = "c";
        re2c:define:YYSETCONDITION = "c = @@;";
        re2c:yyfill:enable = 0;
        re2c:tags = 1;

        end = [\x00];
        mon = 'mon''day'?;
        tue = 'tue''sday'?;
        wed = 'wed''nesday'?;
        thu = 'thu''rsday'?;
        fri = 'fri''day'?;
        sat = 'sat''urday'?;
        sun = 'sun''day'?;
        wday = mon | tue | wed | thu | fri | sat | sun;

        <timer> 'minutely' end
            { td->hours[1] = 23; td->minutes[1] = 59; return true; }
        <timer> 'hourly' end    { td->hours[1] = 23; return true; }
        <timer> ('daily' | 'midnight') end
            { return true; }
        <timer> 'monthly' end   { td->days[1] = 1; return true; }
        <timer> 'weekly' end    { td->wDay = 1; return true; }
        <timer> ('yearly' | 'annually') end
            { td->days[1] = 1; td->month[1] = 1; return true; }

        <timer> '' / wday | '*' { goto yyc_weekday1; }

        <timer> '' / [0-9*.]{1,12} ('-' [0-9*.]{1,6}){2} end
            { goto yyc_year; }
        <timer> '' / [0-9*.]{1,12} ('-' [0-9*.]{1,6}){3} ':'
            { goto yyc_year; }
        <timer> '' /  [0-9*.]{1,6} ':'
            { goto yyc_hour; }

        <weekday1> '*'
            { goto yyc_weekdayend; }
        <weekday1> mon
            { wDay = 1 << 0; goto yyc_weekday2; }
        <weekday1> tue
            { wDay = 1 << 1; goto yyc_weekday2; }
        <weekday1> wed
            { wDay = 1 << 2; goto yyc_weekday2; }
        <weekday1> thu
            { wDay = 1 << 3; goto yyc_weekday2; }
        <weekday1> fri
            { wDay = 1 << 4; goto yyc_weekday2; }
        <weekday1> sat
            { wDay = 1 << 5; goto yyc_weekday2; }
        <weekday1> sun
            { wDay = 1 << 6; goto yyc_weekday2; }

        <weekday2> '' / [0-9*.]{1,12} ('-' [0-9*.]{1,6}){2}
            { goto yyc_year; }
        <weekday2> '' /  [0-9*.]{1,6} ':'
            { goto yyc_hour; }
        <weekday2> end
            { result = result && crinitTimerSetWeekdays(wDay, -1, td); return result; }
        <weekday2> '-'
            { result = result && crinitTimerSetWeekdays(wDay, wDay, td); goto yyc_year; }
        <weekday2> '..' mon
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 0, td); goto yyc_weekdayend; }
        <weekday2> '..' tue
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 1, td); goto yyc_weekdayend; }
        <weekday2> '..' wed
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 2, td); goto yyc_weekdayend; }
        <weekday2> '..' thu
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 3, td); goto yyc_weekdayend; }
        <weekday2> '..' fri
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 4, td); goto yyc_weekdayend; }
        <weekday2> '..' sat
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 5, td); goto yyc_weekdayend; }
        <weekday2> '..' sun
            { result = result && crinitTimerSetWeekdays(wDay, 1 << 6, td); goto yyc_weekdayend; }

        <weekdayend> end
            { return result; }
        <weekdayend> '-'
            { goto yyc_year; }
        <weekdayend> '-' / [0-9*.]{1,6} ':'
            { goto yyc_hour; }

        <year> @t1 [0-9]{0,5} @t2 '..' @t3 [0-9]{0,5} @t4 '-'
            { result = result && crinitTimerSetYears(t1, t2, t3, t4, td); goto yyc_month; }
        <year> @t1 @t3 [0-9]{1,4} @t2 @t4 '-'
            { result = result && crinitTimerSetYears(t1, t2, t3, t4, td); goto yyc_month; }
        <year> @t1 @t2 '*' @t3 @t4 '-'
            { result = result && crinitTimerSetYears(t1, t2, t3, t4, td); goto yyc_month; }

        <year> '' / ([0-9.*]{1,6} ':')                              { goto yyc_hour; }

        <month> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4 '-'
            { result = result && crinitTimerSetMonth(t1, t2, t3, t4, td); goto yyc_day; }
        <month> @t1 @t3 [0-9]{1,2} @t2 @t4 '-'
            { result = result && crinitTimerSetMonth(t1, t2, t3, t4, td); goto yyc_day; }
        <month> @t1 @t2 '*' @t3 @t4 '-'
            { result = result && crinitTimerSetMonth(t1, t2, t3, t4, td); goto yyc_day; }

        <day> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4 end
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); return result; }
        <day> @t1 @t3 [0-9]{1,2} @t2 @t4 end
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); return result; }
        <day> @t1 @t2 '*' @t3 @t4 end
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); return result; }

        <day> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4 '-'
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); goto yyc_hour; }
        <day> @t1 @t3 [0-9]{1,2} @t2 @t4 '-'
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); goto yyc_hour; }
        <day> @t1 @t2 '*' @t3 @t4 '-'
            { result = result && crinitTimerSetDays(t1, t2, t3, t4, td); goto yyc_hour; }


        <hour> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4  ":"
            { result = result && crinitTimerSetHours(t1, t2, t3, t4, td); goto yyc_minute; }
        <hour> @t1 @t3 [0-9]{1,2} @t2 @t4 ":"
            { result = result && crinitTimerSetHours(t1, t2, t3, t4, td); goto yyc_minute; }
        <hour> @t1 @t2 '*' @t3 @t4 ":"
            { result = result && crinitTimerSetHours(t1, t2, t3, t4, td); goto yyc_minute; }

        <minute> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_timezone; }
        <minute> @t1 @t3 [0-9]{1,2} @t2 @t4
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_timezone; }
        <minute> @t1 @t2 '*' @t3 @t4
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_timezone; }

        <minute> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4 ':'
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_second; }
        <minute> @t1 @t3 [0-9]{1,2} @t2 @t4 ':'
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_second; }
        <minute> @t1 @t2 '*' @t3 @t4 ':'
            { result = result && crinitTimerSetMinutes(t1, t2, t3, t4, td); goto yyc_second; }


        <second> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); return result; }
        <second> @t1 @t3 [0-9]{1,2} @t2 @t4
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); return result; }
        <second> @t1 @t2 '*' @t3 @t4
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); return result; }

        <second> @t1 [0-9]{0,2} @t2 '..' @t3 [0-9]{0,2} @t4 / [+-]
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); goto yyc_timezone; }
        <second> @t1 @t3 [0-9]{1,2} @t2 @t4 / [+-]
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); goto yyc_timezone; }
        <second> @t1 @t2 '*' @t3 @t4 / [+-]
            { result = result && crinitTimerSetSeconds(t1, t2, t3, t4, td); goto yyc_timezone; }

        <timezone> end                                              { return result; }
        <timezone> @t1 [+-][0-9]{1,2} @t2 ':'? @t3 ([0-9]{2})? @t4 end
            { result = result && crinitTimerSetTimezone(t1, t2, t3, t4, td); return result; }

        <*>  @t1 * @t2
            { crinitErrPrint("Could not parse timer from '%s'", s); return false; }
    */
}
