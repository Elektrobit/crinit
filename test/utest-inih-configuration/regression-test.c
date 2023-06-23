/**
 * @file regression-test.c
 * @brief Implementation of a regression test to check the compile-time configuration of libinih.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <string.h>

#include "common.h"
#include "ini.h"
#include "unit_test.h"
#include "utest-inih-configuration.h"

#define STR_CHECK_INLINE "CHECK_INLINE"
#define STR_SEMICOLON_LINE "this line should not ; be cut at the semicolon"

static int crinitIniHandler(void *userP, const char *section, const char *name, const char *value);

void crinitInihConfigurationRegressionTest(void **state) {
    CRINIT_PARAM_UNUSED(state);

    // Check compile time defines as specified in deps/inih/README.md
    assert_int_equal(INI_MAX_LINE, 4096);
    assert_int_equal(INI_ALLOW_INLINE_COMMENTS, 0);

    // Check comment behavior
    const char *valid = STR_CHECK_INLINE
        " = " STR_SEMICOLON_LINE
        "\n; this comment line starts with a semicolon\n# this comment line starts with a number sign\n";

    // This should go through fine.
    assert_int_equal(ini_parse_string(valid, crinitIniHandler, NULL), 0);
 }

static int crinitIniHandler(void *userP, const char *section, const char *name, const char *value) {
    CRINIT_PARAM_UNUSED(userP);
    CRINIT_PARAM_UNUSED(section);

    // Check if we're getting the full line without the potential inline comment cut off.
    if (strcmp(name, STR_CHECK_INLINE) == 0 && strcmp(value, STR_SEMICOLON_LINE) == 0) {
        return 1;
    }
    return 0;
}

