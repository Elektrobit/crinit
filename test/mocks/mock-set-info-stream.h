/**
 * @file mock-set-info-stream.h
 * @brief Header declaring a mock function for EBCL_setInfoStream().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_SET_INFO_STREAM_H__
#define __MOCK_SET_INFO_STREAM_H__

#include "logio.h"

/**
 * Mock function for EBCL_setInfoStream().
 *
 * Checks that the right parameters are given through the cmocka API.
 */
void __wrap_EBCL_setInfoStream(FILE *stream);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linkerw
                                               // wrapping.

#endif /* __MOCK_SET_INFO_STREAM_H__ */
