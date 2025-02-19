// SPDX-License-Identifier: MIT
/**
 * @file mock-set-err-stream.h
 * @brief Header declaring a mock function for crinitSetErrStream().
 */
#ifndef __MOCK_SET_ERR_STREAM_H__
#define __MOCK_SET_ERR_STREAM_H__

#include "logio.h"

/**
 * Mock function for crinitSetErrStream().
 *
 * Checks that the right parameters are given through the cmocka API.
 */
void __wrap_crinitSetErrStream(FILE *stream);  // NOLINT(readability-identifier-naming)
                                               // Rationale: Naming scheme fixed due to linkerw
                                               // wrapping.

#endif /* __MOCK_SET_ERR_STREAM_H__ */
