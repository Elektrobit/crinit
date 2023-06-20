/**
 * @file utest-confconv-envset.h
 * @brief Header declaring the unit tests for crinitConfConvToEnvSetMember().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_ENVSET_PARSE_AND_SET_H__
#define __UTEST_ENVSET_PARSE_AND_SET_H__

/**
 * Tests successful parsing of an environment variable definition.
 */
void crinitConfConvToEnvSetMemberTestSuccess(void **state);
/**
 * Tests unsuccessful parsing of an environment variable definition due to invalid syntax.
 */
void crinitConfConvToEnvSetMemberTestWrongInput(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void crinitConfConvToEnvSetMemberTestNullInput(void **state);

#endif /* __UTEST_ENVSET_PARSE_AND_SET_H__ */
