// SPDX-License-Identifier: MIT
#ifndef __TIMER_H__
#define __TIMER_H__

#include <bits/types/struct_itimerspec.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * The type of a timer definition.
 */
typedef struct crinitTimerDef {
    uint8_t wDay;
    uint16_t years[2];
    uint8_t month[2];
    uint8_t days[2];
    uint8_t hours[2];
    uint8_t minutes[2];
    uint8_t seconds[2];
    int8_t timezone[2];
} crinitTimerDef_t;

/**
 * The type of a crinit timer object.
 */
typedef struct crinitTimer {
    crinitTimerDef_t def;
    char *name;
    size_t refs;
    struct itimerspec next;
} crinitTimer_t;

/**
 * Set a timer to the default value:
 * Mon..Sun-0000..65535-01..12-01..31-00:00:00+0000
 *
 * @param td  the crinitTimerDef_t to set to default
 */
void crinitTimerSetDefault(crinitTimerDef_t *td);

/**
 * Check if a timer definition gives a valid timer
 * ie. month between 1 and 12, day in 1 to 31, hours 0 to 23, ...
 *
 * @param td  the timer definition to set check
 *
 * @return true if the timer definition is valid, and false if not
 */
bool crinitCheckTimerDef(crinitTimerDef_t *td);

/**
 * Check if a value is in a closed-open range (test in [start, end) ) ie start <= test < end.
 *
 * @param start   the lower end of the range to check
 * @param test    the value to check if it's in range
 * @param end     the upper end of the range to check
 *
 * @return true if test is higher or equal to start and lower then end
 */
#define CO_RANGE(start, test, end) (((start) <= (test)) && ((test) < (end)))
/**
 * Check if a value is in a closed-closed range (test in [start, end] ) ie start <= test <= end.
 *
 * @param start   the lower end of the range to check
 * @param test    the value to check if it's in range
 * @param end     the upper end of the range to check
 *
 * @return true if test is higher or equal to start and lower or equal to end
 */
#define CC_RANGE(start, test, end) (((start) <= (test)) && ((test) <= (end)))
/**
 * Check if a value is in a open-closed range (test in (start, end] ) ie start < test <= end.
 *
 * @param start   the lower end of the range to check
 * @param test    the value to check if it's in range
 * @param end     the upper end of the range to check
 *
 * @return true if test is higher then start and lower or equal to end
 */
#define OC_RANGE(start, test, end) (((start) < (test)) && ((test) <= (end)))
/**
 * Check if a value is in a open-open range (test in (start, end) ) ie start < test < end.
 *
 * @param start   the lower end of the range to check
 * @param test    the value to check if it's in range
 * @param end     the upper end of the range to check
 *
 * @return true if test is higher then start and lower then end
 */
#define OO_RANGE(start, test, end) (((start) < (test)) && ((test) < (end)))

/**
 * print a crinitTimerDef_t into a string
 *
 * @param s   the string to print to
 * @param td  the crinitTimerDef_t to print
 *
 * @return the number of bytes written to s excluding the terminating 0 byte
 */
int crinitSPrintTimerDef(char *s, crinitTimerDef_t *td);
/**
 * print a crinitTimerDef_t as crinitInfoPrint message
 *
 * @param td  the crinitTimerDef_t to print
 */
void crinitPrintTimerDef(crinitTimerDef_t *td);

/**
 * Parses a timer definition from a string
 *
 * @param s   the string to parse from
 * @param td  the crinitTimerDef_t to set
 *
 * @return true on success, false otherwise
 */
bool crinitTimerParse(char *s, crinitTimerDef_t *td);

/**
 * Calculate the next time the timer should trigger.
 *
 * @param last  the last timestamp to calculate the next from
 * @param td    the timer definition to calculate the next time from
 *
 * @return the timestamp the timer is fullfiled next
 */
struct timespec crinitTimerNextTime(struct timespec *last, crinitTimerDef_t *td);

/**
 * Get a `struct tm` similar to `gmtime_r` with a specific timezone.
 *
 * @param time      the timestamp to transform into a tm
 * @param timezone  the timezone with hour and minute
 * @param result    the resulting tm struct
 *
 * @return  a tm representation in the specyfied timezone
 */
struct tm *crinitZonedTimeR(const time_t *time, int8_t timezone[2], struct tm *restrict result);
/**
 * Check if a timerstamp is valid for a timer
 *
 * @param ts  the timestamp to check.
 * @param td  the crinit timer definition to check against
 *
 * @return true if the timer should trigger at that timestamp, false otherwise
 */
bool crinitCheckTimerTime(struct timespec ts, crinitTimerDef_t *td);

#endif  // __TIMER_H__
