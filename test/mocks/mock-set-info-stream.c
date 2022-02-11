/**
 * @file mock-set-info-stream.c
 * @brief Implementation of a mock function for EBCL_setInfoStream().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "mock-set-info-stream.h"

#include "unit_test.h"

void __wrap_EBCL_setInfoStream(FILE *stream) {  // NOLINT(readability-identifier-naming)
                                                // Rationale: Naming scheme fixed due to linkerw
                                                // wrapping.
    check_expected(stream);
}
