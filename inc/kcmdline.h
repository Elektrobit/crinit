// SPDX-License-Identifier: MIT
/**
 * @file kcmdline.h
 * @brief Header related to working with the Kernel command line.
 */
#ifndef __KCMDLINE_H__
#define __KCMDLINE_H__

#define CRINIT_KCMDLINE_PATH_DEFAULT "/proc/cmdline"  ///< Default path to read the Kernel cmdline from.
#define CRINIT_KCMDLINE_MAX_LEN 4096uL                ///< Maximum supported length of the Kernel command line.

/**
 * Parses and handles crinit-specific options in the Kernel command line.
 *
 * Kernel command line must be accessible as a file at \a cmdlinePath. If the function encounters a setting from
 * crinitKCmdlineConfigMap, it will call the associated handler.
 *
 * @param cmdlinePath  Where to read the Kernel command line from. This should be #CRINIT_KCMDLINE_PATH_DEFAULT.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitKernelCmdlineParse(const char *cmdlinePath);

#endif /* __KCMDLINE_H__ */
