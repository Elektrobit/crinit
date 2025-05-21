// SPDX-License-Identifier: MIT
/**
 * @file capabilities.h
 * @brief Header defining functions to modify capabilities.
 */
#ifndef __CAPABILITIES_H__
#define __CAPABILITIES_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/capability.h>
#include <sys/syscall.h>

/**
 * Convert capability names to integral value and set it in bitmask.

 @param bitmask Bitmask to maintain capability settings.
 @param capabilities Array of capability names
 @return 0 if each capability could be converted and set in the bitmask, -1 otherwise.
*/
int crinitCapConvertToBitmask(uint64_t *bitmask, const char *capabilities);

/**
 * Get a process' inheritable capability set
 *
 * @param pid The ID of the process whose capabilities are retrieved.
 * @param result Pointer to result bitmask that holds the inheritable capability set.
 *
 * @return 0 on success, -1 on error
 */
int crinitCapGetInheritable(pid_t pid, uint64_t *result);

/**
 * Test if a capability is set in a process' effective capability set.
 *
 * @param cap    The capability to test.
 * @param pid    The ID of the process whose capabilities are checked.
 *
 * @return true if capability is set, false else
 */
bool crinitCapIsCapsetEffective(cap_value_t cap, pid_t pid);

/**
 * Set a process' ambient capability set
 *
 * @param capMask The capability set encoded as a bitmask
 *
 * @return 0 on success, -1 on error
 */
int crinitCapSetAmbient(uint64_t capMask);

/**
 * Set a process' inheritable capability set
 *
 * @param capMask The capability set encoded as a bitmask
 *
 * @return 0 on success, -1 on error
 */
int crinitCapSetInheritable(uint64_t capMask);

/**
 * Configure a process to retain its permitted set of capabilities.
 *
 * @return 0 on success, -1 on error
 */
int crinitCapRetainPermitted();

#endif /* __CAPABILITIES_H__ */
