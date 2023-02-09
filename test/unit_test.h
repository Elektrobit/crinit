/**
 * @file unit_test.h
 * @brief Header for common definitions/includes specifically for cmocka unit tests.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
//     Rationale: Specific order of includes needed by cmocka.h.

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

/**
 * Determine whether a function parameter is correct.
 *
 * This ensures the next value queued by one of the expect_*() macros matches
 * the specified value. In contrast to check_expected(), this function takes
 * the parameter name as a string that can be created at runtime.
 *
 * This function needs to be called in the mock object.
 */
#define check_expected_dynamic(parameter_string, value) \
    _check_expected(__func__, parameter_string, __FILE__, __LINE__, cast_to_largest_integral_type(value))

/**
 * CMocka macro to be used to mock return paramters.
 */
#define will_set_parameter(METHOD, PARAM_NAME, VALUE) will_return(METHOD, VALUE)

#endif /* __UNIT_TEST_H__ */
