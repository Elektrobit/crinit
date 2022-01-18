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

#include "globopt.h"

/** Holds the Prefix to put in front of every printed line, defaults to #EBCL_CRINIT_PRINT_PREFIX **/
static char EBCL_printPrefix[EBCL_PRINT_PREFIX_MAX_LEN] = EBCL_CRINIT_PRINT_PREFIX;

static FILE *EBCL_infoStream = NULL;  ///< holds the stream to use for info messages.
static FILE *EBCL_errStream = NULL;   ///< holds the stream to use for error messages.

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

int EBCL_dbgInfoPrint(const char *format, ...) {
    bool globOptDbg = false;
    if (EBCL_globOptGetBoolean(EBCL_GLOBOPT_DEBUG, &globOptDbg) == -1) {
        EBCL_errPrint("Could not read value for \'DEBUG\' from global options, assuming default.");
        globOptDbg = EBCL_GLOBOPT_DEFAULT_DEBUG;
    }
    if (!globOptDbg) {
        return 0;
    }
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_infoStream == NULL) {
        EBCL_infoStream = stdout;
    }
    if ((ret = fprintf(EBCL_infoStream, "%s", EBCL_printPrefix)) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(EBCL_infoStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(EBCL_infoStream, "\n")) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&EBCL_logLock);
    return cCount;
}

int EBCL_infoPrint(const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_infoStream == NULL) {
        EBCL_infoStream = stdout;
    }
    if ((ret = fprintf(EBCL_infoStream, "%s", EBCL_printPrefix)) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(EBCL_infoStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(EBCL_infoStream, "\n")) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&EBCL_logLock);
    return cCount;
}

int EBCL_errPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_errStream == NULL) {
        EBCL_errStream = stderr;
    }
    if ((ret = fprintf(EBCL_errStream, "%s(%s:%s:%d) Error: ", EBCL_printPrefix, file, func, line)) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(EBCL_errStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(EBCL_errStream, "\n")) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&EBCL_logLock);
    return cCount;
}

int EBCL_errnoPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&EBCL_logLock);
    if (EBCL_errStream == NULL) {
        EBCL_errStream = stderr;
    }
    if ((ret = fprintf(EBCL_errStream, "%s(%s:%s:%d) Error: ", EBCL_printPrefix, file, func, line)) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(EBCL_errStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(EBCL_errStream, " Errno: %s\n", EBCL_threadSafeStrerror(errno))) < 0) {
        pthread_mutex_unlock(&EBCL_logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&EBCL_logLock);
    return cCount;
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
