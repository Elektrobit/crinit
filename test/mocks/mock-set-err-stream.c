/**
 * @file mock-set-err-stream.c
 * @brief Implementation of a mock function for crinitSetErrStream().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-set-err-stream.h"

#include "unit_test.h"

void __wrap_crinitSetErrStream(FILE *stream) {  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linkerw
                                               // wrapping.
    check_expected(stream);
}
