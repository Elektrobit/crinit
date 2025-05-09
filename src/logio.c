// SPDX-License-Identifier: MIT
/**
 * @file logio.c
 * @brief Implementation of debug/log output.
 */
#include "logio.h"

#include <locale.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "globopt.h"

#define CRINIT_ERROR_WITH_FFL_FMT "(%s:%s:%d) Error: "  ///< Format string error prefix with function, file and line.
#define CRINIT_ERRNO_FMT " Errno: %s"                   ///< Format string errno suffix, to use with strerror().
#define CRINIT_SYSLOG_IDENT "crinit"                    ///< Identification string for crinit logging to syslog.

/** Holds the Prefix to put in front of every printed line, defaults to #CRINIT_PRINT_PREFIX **/
static char crinitPrintPrefix[CRINIT_PRINT_PREFIX_MAX_LEN] = CRINIT_PRINT_PREFIX;

static FILE *crinitInfoStream = NULL;  ///< holds the stream to use for info messages.
static FILE *crinitErrStream = NULL;   ///< holds the stream to use for error messages.

static bool crinitUseSyslog = false;  ///< specifies if we should use syslog calls instead of FILE streams.

/** Mutex synchronizing print output (so that print statements are atomic wrt. to each other). **/
static pthread_mutex_t crinitLogLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread-safe implementation of strerror() using strerror_l().
 *
 * Always uses the POSIX/C locale to format the readable output.
 *
 * @param errnum  The error number to explain.
 *
 * @returns Pointer to a thread-local static memory location with a fitting error message.
 */
static char *crinitThreadSafeStrerror(int errnum);

void crinitSetPrintPrefix(const char *prefix) {
    pthread_mutex_lock(&crinitLogLock);
    strncpy(crinitPrintPrefix, (prefix == NULL) ? CRINIT_PRINT_PREFIX : prefix, CRINIT_PRINT_PREFIX_MAX_LEN);
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitSetInfoStream(FILE *stream) {
    pthread_mutex_lock(&crinitLogLock);
    crinitInfoStream = (stream == NULL) ? stdout : stream;
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitSetErrStream(FILE *stream) {
    pthread_mutex_lock(&crinitLogLock);
    crinitErrStream = (stream == NULL) ? stderr : stream;
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitSetUseSyslog(bool sl) {
    pthread_mutex_lock(&crinitLogLock);
    if (sl && !crinitUseSyslog) {
        openlog(CRINIT_SYSLOG_IDENT, LOG_CONS, LOG_DAEMON);
    } else if (!sl && crinitUseSyslog) {
        closelog();
    }
    crinitUseSyslog = sl;
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitDbgInfoPrint(const char *format, ...) {
    bool globOptDbg = false;
    if (crinitGlobOptGet(CRINIT_GLOBOPT_DEBUG, &globOptDbg) == -1) {
        crinitErrPrint("Could not read value for \'DEBUG\' from global options, assuming default.");
        globOptDbg = CRINIT_CONFIG_DEFAULT_DEBUG;
    }
    if (!globOptDbg) {
        return;
    }
    va_list args;
    pthread_mutex_lock(&crinitLogLock);
    if (crinitInfoStream == NULL) {
        crinitInfoStream = stdout;
    }
    if (crinitUseSyslog) {
        va_start(args, format);
        vsyslog(LOG_DEBUG | LOG_DAEMON, format, args);
        va_end(args);
    } else {
        fprintf(crinitInfoStream, "%s", crinitPrintPrefix);
        va_start(args, format);
        vfprintf(crinitInfoStream, format, args);
        va_end(args);
        fprintf(crinitInfoStream, "\n");
    }
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitInfoPrint(const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&crinitLogLock);
    if (crinitInfoStream == NULL) {
        crinitInfoStream = stdout;
    }
    if (crinitUseSyslog) {
        va_start(args, format);
        vsyslog(LOG_INFO | LOG_DAEMON, format, args);
        va_end(args);
    } else {
        fprintf(crinitInfoStream, "%s", crinitPrintPrefix);
        va_start(args, format);
        vfprintf(crinitInfoStream, format, args);
        va_end(args);
        fprintf(crinitInfoStream, "\n");
    }
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitErrPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&crinitLogLock);
    if (crinitErrStream == NULL) {
        crinitErrStream = stderr;
    }
    if (crinitUseSyslog) {
        size_t n = snprintf(NULL, 0, CRINIT_ERROR_WITH_FFL_FMT "%s", file, func, line, format) + 1;
        char *syslogFmt = malloc(n);
        if (syslogFmt == NULL) {
            return;
        }
        snprintf(syslogFmt, n, CRINIT_ERROR_WITH_FFL_FMT "%s", file, func, line, format);
        va_start(args, format);
        vsyslog(LOG_ERR | LOG_DAEMON, syslogFmt, args);
        va_end(args);
        free(syslogFmt);
    } else {
        fprintf(crinitErrStream, "%s" CRINIT_ERROR_WITH_FFL_FMT, crinitPrintPrefix, file, func, line);
        va_start(args, format);
        vfprintf(crinitErrStream, format, args);
        va_end(args);
        fprintf(crinitErrStream, "\n");
    }
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitErrnoPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    va_list args;
    int locErrno = errno;
    pthread_mutex_lock(&crinitLogLock);
    if (crinitErrStream == NULL) {
        crinitErrStream = stderr;
    }
    if (crinitUseSyslog) {
        size_t n = snprintf(NULL, 0, CRINIT_ERROR_WITH_FFL_FMT "%s" CRINIT_ERRNO_FMT, file, func, line, format,
                            crinitThreadSafeStrerror(locErrno)) +
                   1;
        char *syslogFmt = malloc(n);
        if (syslogFmt == NULL) {
            return;
        }
        snprintf(syslogFmt, n, CRINIT_ERROR_WITH_FFL_FMT "%s" CRINIT_ERRNO_FMT, file, func, line, format,
                 crinitThreadSafeStrerror(locErrno));
        va_start(args, format);
        vsyslog(LOG_ERR | LOG_DAEMON, syslogFmt, args);
        va_end(args);
        free(syslogFmt);
    } else {
        fprintf(crinitErrStream, "%s" CRINIT_ERROR_WITH_FFL_FMT, crinitPrintPrefix, file, func, line);
        va_start(args, format);
        vfprintf(crinitErrStream, format, args);
        va_end(args);
        fprintf(crinitErrStream, CRINIT_ERRNO_FMT, crinitThreadSafeStrerror(locErrno));
        fprintf(crinitErrStream, "\n");
    }
    pthread_mutex_unlock(&crinitLogLock);
}

void crinitInitKmsgLogging(void) {
    FILE *kmsg = fopen("/dev/kmsg", "w");
    if (kmsg == NULL) {
        pthread_mutex_lock(&crinitLogLock);
        crinitInfoStream = stdout;
        crinitErrStream = stderr;
        pthread_mutex_unlock(&crinitLogLock);
        crinitErrnoPrint("Error openeing /dev/kmsg.");
        return;
    }
    setvbuf(kmsg, NULL, _IOLBF, 128);
    pthread_mutex_lock(&crinitLogLock);
    crinitInfoStream = kmsg;
    crinitErrStream = kmsg;
    pthread_mutex_unlock(&crinitLogLock);
}

static char *crinitThreadSafeStrerror(int errNum) {
    char *ret = NULL;
    locale_t errLoc = newlocale(LC_ALL_MASK, "POSIX", (locale_t)0);
    if (errLoc == (locale_t)0) {
        return ret;
    }
    ret = strerror_l(errNum, errLoc);
    freelocale(errLoc);
    return ret;
}
