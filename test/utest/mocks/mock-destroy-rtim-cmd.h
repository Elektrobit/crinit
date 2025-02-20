// SPDX-License-Identifier: MIT
/**
 * @file mock-destroy-rtim-cmd.h
 * @brief Header declaring a mock function for crinitDestroyRtimCmd().
 */
#ifndef __MOCK_DESTROY_RTIM_CMD_H__
#define __MOCK_DESTROY_RTIM_CMD_H__

#include "rtimcmd.h"

/**
 * Mock function for crinitDestroyRtimCmd().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API.
 * Otherwise the function is a no-op.
 */
int __wrap_crinitDestroyRtimCmd(crinitRtimCmd_t *c);  // NOLINT(readability-identifier-naming)
                                                      // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_DESTROY_RTIM_CMD_H__ */
