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

#include "crinit.h"
#include "globopt.h"

static FILE *infoStream = NULL;  ///< holds the stream to use for info messages.
static FILE *errStream = NULL;   ///< holds the stream to use for error messages.

/** Mutex synchronizing print output (so that print statements are atomic wrt. to each other). **/
static pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread-safe implementation of strerror() using strerror_l().
 *
 * Always uses the POSIX/C locale to format the readable output.
 *
 * @param errnum  The error number to explain.
 *
 * @returns Pointer to a thread-local static memory location with a fitting error message.
 */
static char *threadSafeStrerror(int errnum);

void EBCL_setInfoStream(FILE *stream) {
    pthread_mutex_lock(&logLock);
    infoStream = (stream == NULL) ? stdout : stream;
    pthread_mutex_unlock(&logLock);
}

void EBCL_setErrStream(FILE *stream) {
    pthread_mutex_lock(&logLock);
    errStream = (stream == NULL) ? stderr : stream;
    pthread_mutex_unlock(&logLock);
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
    pthread_mutex_lock(&logLock);
    if (infoStream == NULL) {
        infoStream = stdout;
    }
    if ((ret = fprintf(infoStream, EBCL_CRINIT_PRINT_PREFX)) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(infoStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(infoStream, "\n")) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&logLock);
    return cCount;
}

int EBCL_infoPrint(const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&logLock);
    if (infoStream == NULL) {
        infoStream = stdout;
    }
    if ((ret = fprintf(infoStream, EBCL_CRINIT_PRINT_PREFX)) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(infoStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(infoStream, "\n")) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&logLock);
    return cCount;
}

int EBCL_errPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&logLock);
    if (errStream == NULL) {
        errStream = stderr;
    }
    if ((ret = fprintf(errStream, EBCL_CRINIT_PRINT_PREFX "(%s:%s:%d) Error: ", file, func, line)) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(errStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(errStream, "\n")) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&logLock);
    return cCount;
}

int EBCL_errnoPrintFFL(const char *file, const char *func, int line, const char *format, ...) {
    int ret = 0;
    int cCount = 0;
    va_list args;
    pthread_mutex_lock(&logLock);
    if (errStream == NULL) {
        errStream = stderr;
    }
    if ((ret = fprintf(errStream, EBCL_CRINIT_PRINT_PREFX "(%s:%s:%d) Error: ", file, func, line)) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    va_start(args, format);
    if ((ret = vfprintf(errStream, format, args)) < 0) {
        va_end(args);
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    va_end(args);
    cCount += ret;
    if ((ret = fprintf(errStream, " Errno: %s\n", threadSafeStrerror(errno))) < 0) {
        pthread_mutex_unlock(&logLock);
        return ret;
    }
    cCount += ret;
    pthread_mutex_unlock(&logLock);
    return cCount;
}

static char *threadSafeStrerror(int errNum) {
    char *ret = NULL;
    locale_t errLoc = newlocale(LC_ALL_MASK, "POSIX", (locale_t)0);
    if (errLoc == (locale_t)0) {
        return ret;
    }
    ret = strerror_l(errNum, errLoc);
    freelocale(errLoc);
    return ret;
}
