// SPDX-License-Identifier: MIT
/**
 * @file kcmdline.h
 * @brief Header related to working with the Kernel command line.
 */
#ifndef __KCMDLINE_H__
#define __KCMDLINE_H__

#define CRINIT_KCMDLINE_PATH_DEFAULT "/proc/cmdline"  ///< Default path to read the Kernel cmdline from.
#define CRINIT_KCMDLINE_MAX_LEN 4096uL

int crinitKernelCmdlineParse(const char *cmdlinePath);

#endif /* __KCMDLINE_H__ */
