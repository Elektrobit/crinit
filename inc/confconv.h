/**
 * @file confconv.h
 * @brief Definitions related to conversion operations from configuration values to structured data.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <stdbool.h>

#include "envset.h"
#include "ioredir.h"

/**
 * Extract an array of strings from the value mapped to an indexed key in an crinitConfKvList_t.
 *
 * Will split \a confVal along spaces. Will optionally respect quoting using double quotes if \a doubleQuoting is set to
 * true. A dynamically-allocated array-of-strings is returned. If no longer needed it should be freed using
 * crinitFreeArgvArray(). \a numElements will contain the number of strings inside the output array and the output array
 * will be additionally NULL-terminated, same as argc/argv in `main()`.
 *
 * @param numElements    Will contain the number of strings in the output.
 * @param confVal        The string to split.
 * @param doubleQuoting  If true, crinitConfConvToStrArr will respect quoting with double quotes.
 *
 * @return  A new dynamically allocated array of the substrings in \a confVal on success, NULL on error.
 */
char **crinitConfConvToStrArr(int *numElements, const char *confVal, bool doubleQuoting);
/**
 * Initializes an instance of crinitIoRedir_t from an IO redirection statement in a string.
 *
 * The string must be of the form
 * ```
 * <REDIRECT_FROM> <REDIRECT_TO> [ APPEND | TRUNCATE ] [ OCTAL MODE ]
 * ```
 * Where REDIRECT_FROM is one of STDOUT, STDERR, STDIN and REDIRECT_TO may either also be one of those streams or an
 * absolute path to a file. APPEND or TRUNCATE signify whether an existing file at that location should be appended to
 * or truncated. Default ist TRUNCATE. OCTAL MODE sets the permission bits if the file is newly created. Default is
 * 0644.
 *
 * The function may allocate memory inside the crinitIoRedir_t struct which must be freed using crinitDestroyIoRedir().
 *
 * @param ior      The crinitIoRedir_t instance to initialize.
 * @param confVal  The string with the statement to parse.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitConfConvToIoRedir(crinitIoRedir_t *ior, const char *confVal);

/**
 * Parses a single ENV_SET directive and sets the variable in question accordingly.
 *
 * For details on the syntax, see the relevant section in README.md.
 *
 * @param es       The environment set to be modified, must be initialized.
 * @param confVal  The ENV_SET directive to be parsed and "executed" on the set.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitConfConvToEnvSetMember(crinitEnvSet_t *es, const char *confVal);

/** Converts a string to a signed integer, see crinitConfConvToInteger() **/
int crinitConfConvToIntegerI(int *x, const char *confVal, int base);
/** Converts a string to an unsigned long long, see crinitConfConvToInteger() **/
int crinitConfConvToIntegerULL(unsigned long long *x, const char *confVal, int base);
/**
 * Type-generic macro for string to integer conversion.
 *
 * Currently only implemented for `int` and `unsigned long long`.
 *
 * @param out      Output pointer, type-generic (but must be a pointer).
 * @param confVal  The string to convert.
 * @param base     The decimal base to use in conversion.
 *
 * @return  0 on success, -1 on error
 */
// clang-format off
// Rationale: Used version of clang-format does not format _Generic macros correctly. This is a known bug and has been
// fixed very recently. We may remove this exemption once we are on the new clang version as standard.
// See: https://github.com/llvm/llvm-project/issues/18080
#define crinitConfConvToInteger(out, confVal, base)           \
    _Generic((*(out)),                                       \
             int : crinitConfConvToIntegerI,                  \
             unsigned long long : crinitConfConvToIntegerULL) \
             (out, confVal, base)
// clang-format on

/**
 * Converts a string to bool.
 *
 * String must be equal to either `NO` (`==false`) or `YES` (`==true`).
 *
 * @param b        Output pointer to bool variable.
 * @param confVal  The string to convert.
 *
 * @return  0 on success, -1 on error.
 */
int crinitConfConvToBool(bool *b, const char *confVal);

