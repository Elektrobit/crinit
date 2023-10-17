// SPDX-License-Identifier: MIT
/**
 * @file mock-crinit-xfer.h
 * @brief Implementation of a mock function for crinitXfer().
 */
#ifndef __MOCK_BUILD_RTIM_CMD_H__
#define __MOCK_BUILD_RTIM_CMD_H__

#include "rtimcmd.h"

/**
 * Mock function for crinitXfer().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API.
 * Otherwise the function is a no-op.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_crinitXfer(const char *sockFile, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);

#endif /* __MOCK_BUILD_RTIM_CMD_H__ */
