/**
 * @file common.h
 * @brief Header for common definitions not related to other specific features.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __COMMON_H__
#define __COMMON_H__

/**
 * Suppress unused parameter warning for variable as per coding guideline `[OS_C_UNUSED_010]`.
 *
 * May be needed if an external interface is implemented.
 *
 * @param par  Unused variable that should not be warned about.
 */
#define EBCL_PARAM_UNUSED(par) \
    do {                       \
        (void)(par);           \
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
#define EBCL_paramCheck(inputParam, cmpShort, cmpLong) \
    ((strncmp(inputParam, cmpShort, sizeof(cmpShort)) == 0) || (strncmp(inputParam, cmpLong, sizeof(cmpLong)) == 0))

/**
 * Check if \a path is absolute (i.e. starts with '/').
 *
 * @param path  The path to check.
 *
 * @return true if path is absolute, false otherwise
 */
#define EBCL_isAbsPath(path) (((path) != NULL) && ((path)[0] == '/'))

#endif /* __COMMON_H__ */
