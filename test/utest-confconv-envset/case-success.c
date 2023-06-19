/**
 * @file case-success.c
 * @brief Unit test for crinitConfConvToEnvSetMember(), successful execution.
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

void crinitConfConvToEnvSetMemberTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    const char *vanillaConf = "VANILLA_VAR \"That is tasty.\"", *vanillaRes = "That is tasty.";
    const char *escSeqConf = "ESCSEQ_VAR \"Hello,\\x20fans!\n\"", *escSeqRes = "Hello, fans!\n";
    const char *substConf = "SUBST_VAR \"'${ESCSEQ_VAR}' is a well-known phrase.\"",
               *substRes = "'Hello, fans!\n' is a well-known phrase.";
    const char *combinedConf = "COMPLEX_VAR \"${ESCSEQ_VAR}\\t${VANILLA_VAR}\"",
               *combinedRes = "Hello, fans!\n\tThat is tasty.";

    crinitEnvSet_t e = {NULL, 0, 0};
    assert_int_equal(crinitEnvSetInit(&e, CRINIT_ENVSET_INITIAL_SIZE, CRINIT_ENVSET_SIZE_INCREMENT), 0);

    assert_int_equal(crinitConfConvToEnvSetMember(&e, vanillaConf), 0);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, escSeqConf), 0);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, substConf), 0);
    assert_int_equal(crinitConfConvToEnvSetMember(&e, combinedConf), 0);

    assert_string_equal(crinitEnvSetGet(&e, "VANILLA_VAR"), vanillaRes);
    assert_string_equal(crinitEnvSetGet(&e, "ESCSEQ_VAR"), escSeqRes);
    assert_string_equal(crinitEnvSetGet(&e, "SUBST_VAR"), substRes);
    assert_string_equal(crinitEnvSetGet(&e, "COMPLEX_VAR"), combinedRes);

    assert_int_equal(crinitEnvSetDestroy(&e), 0);
}
