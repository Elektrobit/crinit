/**
 * @file utest-confconv-envset.h
 * @brief Header declaring the unit tests for EBCL_confConvToEnvSetMember().
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
void EBCL_confConvToEnvSetMemberTestSuccess(void **state);
/**
 * Tests unsuccessful parsing of an environment variable definition due to invalid syntax.
 */
void EBCL_confConvToEnvSetMemberTestWrongInput(void **state);
/**
 * Tests detection of NULL pointer input.
 */
void EBCL_confConvToEnvSetMemberTestNullInput(void **state);

#endif /* __UTEST_ENVSET_PARSE_AND_SET_H__ */
