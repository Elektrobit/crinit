// SPDX-License-Identifier: MIT
/**
 * @file mock-build-rtim-cmd.h
 * @brief Header declaring a mock function for crinitBuildRtimCmd().
 */
#ifndef __MOCK_BUILD_RTIM_CMD_H__
#define __MOCK_BUILD_RTIM_CMD_H__

#include "rtimcmd.h"

/**
 * Mock function for crinitBuildRtimCmd().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API.
 * Otherwise the function is a no-op.
 */
// NOLINTNEXTLINE(readability-identifier-naming) Rationale: Naming scheme fixed due to linker wrapping.
int __wrap_crinitBuildRtimCmd(crinitRtimCmd_t *c, crinitRtimOp_t op, int argc, ...);

#endif /* __MOCK_BUILD_RTIM_CMD_H__ */
