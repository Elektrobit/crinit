// SPDX-License-Identifier: MIT
/**
 * @file case-set.c
 * @brief Implementation of a unit test for crinitClientSetInfoStream().
 */
#include "common.h"
#include "crinit-client.h"
#include "mock-set-info-stream.h"
#include "unit_test.h"
#include "utest-crinit-set-info-stream.h"

void crinitClientSetInfoStreamTestSet(void **state) {
    CRINIT_PARAM_UNUSED(state);

    FILE *fp = (FILE *)0x12345678;
    expect_value(__wrap_crinitSetInfoStream, stream, fp);
    crinitClientSetInfoStream(fp);
}
