/**
 * @file logio.h
 * @brief Header related to debug/log output.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __LOGIO_H__
#define __LOGIO_H__

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * Maximum size of prefix set with crinitSetPrintPrefix()
 */
#define CRINIT_PRINT_PREFIX_MAX_LEN 32

/**
 * Default prefix to put in front of log/info/error messages.
 */
#define CRINIT_PRINT_PREFIX "[CRINIT] "

/**
 * Constant to use if one wishes to output an empty line using an output function, e.g.
 * crinitInfoPrint(CRINIT_PRINT_EMPTY_LINE). The output will still contain everything the output function adds, e.g.
 * prefix, line number, etc.
 */
#define CRINIT_PRINT_EMPTY_LINE "%s", ""

/**
 * Constant to use instead of __FILE__ which shows only the filename without the leading path.
 *
 * For gcc from version 12 and clang from version 9, this already exists as a macro called __FILE_NAME__. If we detect
 * a suitable compiler version, we'll use that, otherwise implement it manually by looking at the last slash in the
 * path. Note, that this precludes us from using escaped slashes in file names (which we should not do anyway).
 *
 * Using __builtin_strrchr() on a string constant should cause compile-time evaluation of the expressio.
 */
#if __GNUC__ >= 12 || __clang_major__ >= 9
#define __FILE_BASENAME__ __FILE_NAME__
#else
#define __FILE_BASENAME__ (__builtin_strrchr("/" __FILE__, '/') + 1)
#endif

/**
 * Set prefix to put in front of error and info message lines.
 *
 * @param prefix  The prefix to use.
 */
void crinitSetPrintPrefix(const char *prefix);
/**
 * Set FILE stream to use for info messages.
 *
 * Defaults to stdout. This can be used to divert the info messages into a log file. The stream can be the same as the
 * one used for crinitSetErrStream() if the log should contain both types of messages. The function uses mutexes
 * internally and is thread-safe.
 *
 * Currently, the config file option to make use of this functionality is not yet implemented.
 *
 * @param stream  The FILE* stream to use. If NULL, stream is set to stdout.
 */
void crinitSetInfoStream(FILE *stream);
/**
 * Set FILE stream to use for error messages.
 *
 * Defaults to stderr. This can be used to divert the error messages into a log file. The stream can be the same as the
 * one used for crinitSetInfoStream() if the log should contain both types of messages. The function uses mutexes
 * internally and is thread-safe.
 *
 * Currently, the config file option to make use of this functionality is not yet implemented.
 *
 * @param stream  The FILE* stream to use. If NULL, stream is set to stderr.
 */
void crinitSetErrStream(FILE *stream);
/**
 * Specify if syslog should be used..
 *
 * By default, Crinit will always use the specified FILE streams. If this is set to `true`, however, Crinit will output
 * to syslog instead. The log connection will be opened with `LOG_CONS`, so if the connection fails, output will be sent
 * to the system console instead.
 *
 * @param sl  `true` if syslog should be used, `false` otherwise.
 */
void crinitSetUseSyslog(bool sl);

/**
 * Print an info message.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix at the start and a newline
 * at the end. It uses the output stream specified by crinitSetInfoStream() or stdout if unset. The function uses mutexes
 * internally and is thread-safe.
 *
 */
void crinitInfoPrint(const char *format, ...) __attribute__((format(printf, 1, 2)));
/**
 * Print an info message if DEBUG global option is set.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix at the start and a newline
 * at the end. It uses the output stream specified by crinitSetInfoStream() or stdout if unset. There will be no output
 * if the global option DEBUG is set to false using crinitGlobOptSetBoolean(). The function uses mutexes internally and
 * is thread-safe.
 *
 * If configured (`USE_SYSLOG = true`) and available (a task has provided `syslog`), the function will instead write to
 * syslog.
 */
void crinitDbgInfoPrint(const char *format, ...) __attribute__((format(printf, 1, 2)));

/**
 * Macro to print an error message including the offending source file, function, and line using crinitErrPrintFFL().
 */
#define crinitErrPrint(...) crinitErrPrintFFL(__FILE_BASENAME__, __func__, __LINE__, __VA_ARGS__)
/**
 * Print an error message.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix and the given \a file, \a
 * func, and \a line parameters at the start as well as a newline at the end. It uses the output stream specified by
 * crinitSetErrStream() or stderr if unset. The function uses mutexes internally and is thread-safe.
 *
 * The macro crinitErrPrint() should be used to provide the function with the correct \a file, \a func, and \a line
 * parameters.
 *
 * If configured (`USE_SYSLOG = true`) and available (a task has provided `syslog`), the function will instead write to
 * syslog.
 */
void crinitErrPrintFFL(const char *file, const char *func, int line, const char *format, ...)
    __attribute__((format(printf, 4, 5)));

/**
 * Macro to print an error message using crinitErrnoPrintFFL() including the offending source file, function, and line,
 * as well as the value of errno.
 *
 */
#define crinitErrnoPrint(...) crinitErrnoPrintFFL(__FILE_BASENAME__, __func__, __LINE__, __VA_ARGS__)
/**
 * Print an error message including a text representation of the current value of errno.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix and the given \a file, \a
 * func, and \a line parameters at the start as well as a text representation of errno and a newline at the end.
 * It uses the output stream specified by crinitSetErrStream() or stderr if unset. The function uses mutexes internally
 * and is thread-safe.
 *
 * The macro crinitErrnoPrint() should be used to provide the function with the correct \a file, \a func, and \a line
 * parameters.
 *
 * If configured (`USE_SYSLOG = true`) and available (a task has provided `syslog`), the function will instead write to
 * syslog.
 */
void crinitErrnoPrintFFL(const char *file, const char *func, int line, const char *format, ...)
    __attribute__((format(printf, 4, 5)));

#endif /* __LOGIO_H__ */
