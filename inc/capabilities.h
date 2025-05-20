// SPDX-License-Identifier: MIT
/**
 * @file capabilities.h
 * @brief Header defining functions to modify capabilities.
 */
#ifndef __CAPABILITIES_H__
#define __CAPABILITIES_H__

#include <stdint.h>

/**
 * Convert capability names to integral value and set it in bitmask.

 @param bitmask Bitmask to maintain capability settings.
 @param capabilities Array of capability names
 @return 0 if each capability could be converted and set in the bitmask, -1 otherwise.
*/
int crinitCapConvertToBitmask(uint64_t *bitmask, const char *capabilities);

#endif /* __CAPABILITIES_H__ */
