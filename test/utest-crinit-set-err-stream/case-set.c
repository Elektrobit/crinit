/**
 * @file case-set.c
 * @brief Implementation of a unit test for crinitClientSetErrStream().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "common.h"
#include "crinit-client.h"
#include "mock-set-err-stream.h"
#include "unit_test.h"
#include "utest-crinit-set-err-stream.h"

void crinitClientSetErrStreamTestSet(void **state) {
    CRINIT_PARAM_UNUSED(state);

    FILE *fp = (FILE *)0x12345678;
    expect_value(__wrap_crinitSetErrStream, stream, fp);
    crinitClientSetErrStream(fp);
}
