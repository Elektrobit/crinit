/**
 * @file mock-destroy-rtim-cmd.h
 * @brief Header declaring a mock function for EBCL_destroyRtimCmd().
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MOCK_DESTROY_RTIM_CMD_H__
#define __MOCK_DESTROY_RTIM_CMD_H__

#include "rtimcmd.h"

/**
 * Mock function for EBCL_destroyRtimCmd().
 *
 * Checks that the right parameters are given and returns a pre-set value through the cmocka API.
 * Otherwise the function is a no-op.
 */
int __wrap_EBCL_destroyRtimCmd(ebcl_RtimCmd_t *c);  // NOLINT(readability-identifier-naming)
                                                    // Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_DESTROY_RTIM_CMD_H__ */
