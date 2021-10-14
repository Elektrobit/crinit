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
#include <stdio.h>

/**
 * Set FILE stream to use for info messages.
 *
 * Defaults to stdout. This can be used to divert the info messages into a log file. The stream can be the same as the
 * one used for EBCL_setErrStream() if the log should contain both types of messages. The function uses mutexes
 * internally and is thread-safe.
 *
 * Currently, the config file option to make use of this functionality is not yet implemented.
 *
 * @param stream  The FILE* stream to use. If NULL, stream is set to stdout.
 */
void EBCL_setInfoStream(FILE *stream);
/**
 * Set FILE stream to use for error messages.
 *
 * Defaults to stderr. This can be used to divert the error messages into a log file. The stream can be the same as the
 * one used for EBCL_setInfoStream() if the log should contain both types of messages. The function uses mutexes
 * internally and is thread-safe.
 *
 * Currently, the config file option to make use of this functionality is not yet implemented.
 *
 * @param stream  The FILE* stream to use. If NULL, stream is set to stderr.
 */
void EBCL_setErrStream(FILE *stream);

/**
 * Print an info message.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix at the start and a newline
 * at the end. It uses the output stream specified by EBCL_setInfoStream() or stdout if unset. The function uses mutexes
 * internally and is thread-safe.
 *
 * @return The number of characters printed.
 */
int EBCL_infoPrint(const char *format, ...);
/**
 * Print an info message if DEBUG global option is set.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix at the start and a newline
 * at the end. It uses the output stream specified by EBCL_setInfoStream() or stdout if unset. There will be no output
 * if the global option DEBUG is set to false using EBCL_globOptSetBoolean(). The function uses mutexes internally and
 * is thread-safe.
 *
 * @return The number of characters printed.
 */
int EBCL_dbgInfoPrint(const char *format, ...);

/**
 * Macro to print an error message including the offending source file, function, and line using EBCL_errPrintFFL().
 */
#define EBCL_errPrint(...) EBCL_errPrintFFL(__FILE__, __func__, __LINE__, __VA_ARGS__)
/**
 * Print an error message.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix and the given \a file, \a
 * func, and \a line parameters at the start as well as a newline at the end. It uses the output stream specified by
 * EBCL_setErrStream() or stderr if unset. The function uses mutexes internally and is thread-safe.
 *
 * The macro EBCL_errPrint() should be used to provide the function with the correct \a file, \a func, and \a line
 * parameters.
 *
 * @return The number of characters printed.
 */
int EBCL_errPrintFFL(const char *file, const char *func, int line, const char *format, ...);

/**
 * Macro to print an error message using EBCL_errnoPrintFFL() including the offending source file, function, and line,
 * as well as the value of errno.
 *
 */
#define EBCL_errnoPrint(...) EBCL_errnoPrintFFL(__FILE__, __func__, __LINE__, __VA_ARGS__)
/**
 * Print an error message including a text representation of the current value of errno.
 *
 * Can be used like printf(). In contrast to printf(), this function adds the Crinit prefix and the given \a file, \a
 * func, and \a line parameters at the start as well as a text representation of errno and a newline at the end.
 * It uses the output stream specified by EBCL_setErrStream() or stderr if unset. The function uses mutexes internally
 * and is thread-safe.
 *
 * The macro EBCL_errnoPrint() should be used to provide the function with the correct \a file, \a func, and \a line
 * parameters.
 *
 * @return The number of characters printed.
 */
int EBCL_errnoPrintFFL(const char *file, const char *func, int line, const char *format, ...);

#endif /* __LOGIO_H__ */
