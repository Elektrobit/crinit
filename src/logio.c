/**
 * @file logio.c
 * @brief Implementation of debug/log output.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "logio.h"

#include <locale.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "globopt.h"

#define EBCL_ERROR_WITH_FFL_FMT "(%s:%s:%d) Error: "  ///< Format string error prefix with function, file and line.
#define EBCL_ERRNO_FMT " Errno: %s"                   ///< Format string errno suffix, to use with strerror().
#define EBCL_CRINIT_SYSLOG_IDENT "crinit"             ///< Identification string for crinit logging to syslog.

/** Holds the Prefix to put in front of every printed line, defaults to #EBCL_CRINIT_PRINT_PREFIX **/
static char EBCL_printPrefix[EBCL_PRINT_PREFIX_MAX_LEN] = EBCL_CRINIT_PRINT_PREFIX;

static FILE *EBCL_infoStream = NULL;  ///< holds the stream to use for info messages.
static FILE *EBCL_errStream = NULL;   ///< holds the stream to use for error messages.

static bool EBCL_useSyslog = false;  ///< specifies if we should use syslog calls instead of FILE streams.

/** Mutex synchronizing print output (so that print statements are atomic wrt. to each other). **/
static pthread_mutex_t EBCL_logLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread-safe implementation of strerror() using strerror_l().
 *
 * Always uses the POSIX/C locale to format the readable output.
 *
 * @param errnum  The error number to explain.
 *
 * @returns Pointer to a thread-local static memory location with a fitting error message.
 */
static char *EBCL_threadSafeStrerror(int errnum);

void EBCL_setPrintPrefix(const char *prefix) {
    pthread_mutex_lock(&EBCL_logLock);
    strncpy(EBCL_printPrefix, (prefix == NULL) ? EBCL_CRINIT_PRINT_PREFIX : prefix, EBCL_PRINT_PREFIX_MAX_LEN);
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_setInfoStream(FILE *stream) {
    pthread_mutex_lock(&EBCL_logLock);
    EBCL_infoStream = (stream == NULL) ? stdout : stream;
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_setErrStream(FILE *stream) {
    pthread_mutex_lock(&EBCL_logLock);
    EBCL_errStream = (stream == NULL) ? stderr : stream;
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_setUseSyslog(bool sl) {
    pthread_mutex_lock(&EBCL_logLock);
    if (sl && !EBCL_useSyslog) {
        openlog(EBCL_CRINIT_SYSLOG_IDENT, LOG_CONS, LOG_DAEMON);
    } else if (!sl && EBCL_useSyslog) {
        closelog();
    }
    EBCL_useSyslog = sl;
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_dbgInfoPrint(const char *format, ...) {
    bool globOptDbg = false;
    if (EBCL_globOptGetBoolean(EBCL_GLOBOPT_DEBUG, &globOptDbg) == -1) {
        EBCL_errPrint("Could not read value for \'DEBUG\' from global options, assuming default.");
        globOptDbg = EBCL_GLOBOPT_DEFAULT_DEBUG;
    }
    if (!globOptDbg) {
        return;
    }
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_infoStream == NULL) {
        EBCL_infoStream = stdout;
    }
    if (EBCL_useSyslog) {
        va_start(args, format);
        vsyslog(LOG_DEBUG | LOG_DAEMON, format, args);
        va_end(args);
    } else {
        fprintf(EBCL_infoStream, "%s", EBCL_printPrefix);
        va_start(args, format);
        vfprintf(EBCL_infoStream, format, args);
        va_end(args);
        fprintf(EBCL_infoStream, "\n");
    }
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_infoPrint(const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_infoStream == NULL) {
        EBCL_infoStream = stdout;
    }
    if (EBCL_useSyslog) {
        va_start(args, format);
        vsyslog(LOG_INFO | LOG_DAEMON, format, args);
        va_end(args);
    } else {
        fprintf(EBCL_infoStream, "%s", EBCL_printPrefix);
        va_start(args, format);
        vfprintf(EBCL_infoStream, format, args);
        va_end(args);
        fprintf(EBCL_infoStream, "\n");
    }
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_errPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_errStream == NULL) {
        EBCL_errStream = stderr;
    }
    if (EBCL_useSyslog) {
        size_t n = strlen(EBCL_ERROR_WITH_FFL_FMT) + strlen(format) + 1;
        char *syslogFmt = malloc(n);
        if (syslogFmt == NULL) {
            return;
        }
        snprintf(syslogFmt, n, EBCL_ERROR_WITH_FFL_FMT "%s", file, func, line, format);
        va_start(args, format);
        vsyslog(LOG_ERR | LOG_DAEMON, syslogFmt, args);
        va_end(args);
        free(syslogFmt);
    } else {
        fprintf(EBCL_errStream, "%s" EBCL_ERROR_WITH_FFL_FMT, EBCL_printPrefix, file, func, line);
        va_start(args, format);
        vfprintf(EBCL_errStream, format, args);
        va_end(args);
        fprintf(EBCL_errStream, "\n");
    }
    pthread_mutex_unlock(&EBCL_logLock);
}

void EBCL_errnoPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_errStream == NULL) {
        EBCL_errStream = stderr;
    }
    if (EBCL_useSyslog) {
        size_t n = strlen(EBCL_ERROR_WITH_FFL_FMT) + strlen(format) + strlen(EBCL_ERRNO_FMT) + 1;
        char *syslogFmt = malloc(n);
        if (syslogFmt == NULL) {
            return;
        }
        snprintf(syslogFmt, n, EBCL_ERROR_WITH_FFL_FMT "%s" EBCL_ERRNO_FMT, file, func, line, format,
                 EBCL_threadSafeStrerror(errno));
        va_start(args, format);
        vsyslog(LOG_ERR | LOG_DAEMON, syslogFmt, args);
        va_end(args);
        free(syslogFmt);
    } else {
        fprintf(EBCL_errStream, "%s" EBCL_ERROR_WITH_FFL_FMT, EBCL_printPrefix, file, func, line);
        va_start(args, format);
        vfprintf(EBCL_errStream, format, args);
        va_end(args);
        fprintf(EBCL_errStream, EBCL_ERRNO_FMT, EBCL_threadSafeStrerror(errno));
        fprintf(EBCL_errStream, "\n");
    }
    pthread_mutex_unlock(&EBCL_logLock);
}

static char *EBCL_threadSafeStrerror(int errNum) {
    char *ret = NULL;
    locale_t errLoc = newlocale(LC_ALL_MASK, "POSIX", (locale_t)0);
    if (errLoc == (locale_t)0) {
        return ret;
    }
    ret = strerror_l(errNum, errLoc);
    freelocale(errLoc);
    return ret;
}
