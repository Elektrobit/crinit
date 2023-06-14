/**
 * @file case-success.c
 * @brief Unit test for EBCL_confConvToEnvSetMember(), successful execution.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "confconv.h"
#include "envset.h"
#include "unit_test.h"
#include "utest-confconv-envset.h"

void EBCL_confConvToEnvSetMemberTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *vanillaConf = "VANILLA_VAR \"That is tasty.\"", *vanillaRes = "That is tasty.";
    const char *escSeqConf = "ESCSEQ_VAR \"Hello,\\x20fans!\n\"", *escSeqRes = "Hello, fans!\n";
    const char *substConf = "SUBST_VAR \"'${ESCSEQ_VAR}' is a well-known phrase.\"",
               *substRes = "'Hello, fans!\n' is a well-known phrase.";
    const char *combinedConf = "COMPLEX_VAR \"${ESCSEQ_VAR}\\t${VANILLA_VAR}\"",
               *combinedRes = "Hello, fans!\n\tThat is tasty.";

    ebcl_EnvSet_t e = {NULL, 0, 0};
    assert_int_equal(EBCL_envSetInit(&e, EBCL_ENVSET_INITIAL_SIZE, EBCL_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(EBCL_confConvToEnvSetMember(&e, vanillaConf), 0);
    assert_int_equal(EBCL_confConvToEnvSetMember(&e, escSeqConf), 0);
    assert_int_equal(EBCL_confConvToEnvSetMember(&e, substConf), 0);
    assert_int_equal(EBCL_confConvToEnvSetMember(&e, combinedConf), 0);

    assert_string_equal(EBCL_envSetGet(&e, "VANILLA_VAR"), vanillaRes);
    assert_string_equal(EBCL_envSetGet(&e, "ESCSEQ_VAR"), escSeqRes);
    assert_string_equal(EBCL_envSetGet(&e, "SUBST_VAR"), substRes);
    assert_string_equal(EBCL_envSetGet(&e, "COMPLEX_VAR"), combinedRes);

    assert_int_equal(EBCL_envSetDestroy(&e), 0);
}
