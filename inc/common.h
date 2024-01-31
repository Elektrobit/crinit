// SPDX-License-Identifier: MIT
/**
 * @file common.h
 * @brief Header for common definitions/functions not related to other specific features.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdint.h>

/**
 * Suppress unused parameter warning for variable as per coding guideline `[OS_C_UNUSED_010]`.
 *
 * May be needed if an external interface is implemented.
 *
 * @param par  Unused variable that should not be warned about.
 */
#define CRINIT_PARAM_UNUSED(par) \
    do {                         \
        (void)(par);             \
    } while (0)

/**
 * Checks if a string is equal to at least one of two comparison literals.
 *
 * Meant to be used in a loop to check argv for long and short options. Parameters must not be NULL.
 *
 * @param inputParam  pointer to a c string to be compared, may be const
 * @param cmpShort    first comparison string literal, meant for the short form of the option
 * @param cmpLong     second comparison string literal, meant for the long form of the option
 *
 * @return  true if \a inputParam is lexicographically equal to at least one of \a cmpShort and \a cmpLong, false
 *          otherwise
 */
#define crinitParamCheck(inputParam, cmpShort, cmpLong) \
    ((strncmp(inputParam, cmpShort, sizeof(cmpShort)) == 0) || (strncmp(inputParam, cmpLong, sizeof(cmpLong)) == 0))

/**
 * Check if \a path is absolute (i.e. starts with '/').
 *
 * @param path  The path to check.
 *
 * @return true if path i*s absolute, false otherwise
 */
#define crinitIsAbsPath(path) (((path) != NULL) && ((path)[0] == '/'))

/**
 * Calculate the number of elements of an array at compile-time.
 */
#define crinitNumElements(p) (sizeof(p) / sizeof(*(p)))

/**
 * Macro to simplify checking for null pointer inputs at the start of a function.
 *
 * Will print an error message and return from the calling function with the given error code if any of the given
 * variables are NULL.
 *
 * Declares the variables `_macroPtrsToCheck` and `_macroI` internally. These names must not be used as parameter names
 * to functions which use this macro.
 *
 * @param errcode  The error code to return if \a expr is false. Must be a compatible type to the return type of the
 * encompassing function.
 * @param ...      Variadic list of parameter names to check if they are NULL.
 */
#define crinitNullCheck(errcode, ...)                                                         \
    do {                                                                                      \
        _Pragma("GCC diagnostic push");                                                       \
        _Pragma("GCC diagnostic error \"-Wshadow\"");                                         \
        const void *_macroPtrsToCheck[] = {__VA_ARGS__};                                      \
        for (size_t _macroI = 0; _macroI < crinitNumElements(_macroPtrsToCheck); _macroI++) { \
            if (_macroPtrsToCheck[_macroI] == NULL) {                                         \
                crinitErrPrint("Input parameters must not be NULL.");                         \
                return (errcode);                                                             \
            }                                                                                 \
        }                                                                                     \
        _Pragma("GCC diagnostic pop");                                                        \
    } while (0)

/**
 * Macro for a type-generic implementation of `strto*()`.
 *
 * Example: `unsigned long x = strtoGenericInteger(x, "0xFF", NULL, 16);` will map to
 *          `unsigned long x = strtoul("0xFF", NULL, 16);`.
 */
// clang-format off
// Rationale: Used version of clang-format does not format _Generic macros correctly. This is a known bug and has been
// fixed very recently. We may remove this exemption once we are on the new clang version as standard.
// See: https://github.com/llvm/llvm-project/issues/18080
#define crinitStrtoGenericInteger(resType, str, endptr, base) \
    _Generic((resType),                                       \
             int : strtol,                                    \
             long : strtol,                                   \
             long long : strtoll,                             \
             unsigned int : strtoul,                          \
             unsigned long : strtoul,                         \
             unsigned long long : strtoull)                   \
             ((str), (endptr), (base))
// clang-format on

/**
 * Convenience macro to free memory and also set the pointer to it to NULL.
 *
 * @param ptr  The pointer to be "nullified".
 */
#define crinitNullify(ptr) \
    do {                   \
        free(ptr);         \
        (ptr) = NULL;      \
    } while (0)

/**
 * Reads a whole binary file to memory.
 *
 * Will return an error if the given buffer is too small to read all of the file's contents.
 *
 * @param buf   The buffer where the data will be stored. Should be of sufficient size to hold the file to be read.
 * @param n     The size of the given memory buffer.
 * @param path  Path to the file to be read.
 *
 * @return  0 on success, -1 otherwise.
 */
int crinitBinReadAll(uint8_t *buf, size_t n, const char *path);

#endif /* __COMMON_H__ */
