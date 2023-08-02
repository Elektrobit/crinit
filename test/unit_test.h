// SPDX-License-Identifier: MIT
/**
 * @file unit_test.h
 * @brief Header for common definitions/includes specifically for cmocka unit tests.
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

// workaround for cmocka API changes after 1.1.5
#ifndef cast_to_largest_integral_type
#define cast_to_largest_integral_type(value) cast_to_uintmax_type(value)
#endif

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
