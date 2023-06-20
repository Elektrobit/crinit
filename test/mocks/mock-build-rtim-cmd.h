/**
 * @file mock-build-rtim-cmd.h
 * @brief Header declaring a mock function for crinitBuildRtimCmd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
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
