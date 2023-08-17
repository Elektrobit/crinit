// SPDX-License-Identifier: MIT
/**
 * @file mock-set-info-stream.h
 * @brief Header declaring a mock function for crinitSetInfoStream().
 */
#ifndef __MOCK_SET_INFO_STREAM_H__
#define __MOCK_SET_INFO_STREAM_H__

#include "logio.h"

/**
 * Mock function for crinitSetInfoStream().
 *
 * Checks that the right parameters are given through the cmocka API.
 */
void __wrap_crinitSetInfoStream(FILE *stream);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linkerw
                                               // wrapping.

#endif /* __MOCK_SET_INFO_STREAM_H__ */
