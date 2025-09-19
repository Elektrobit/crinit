// SPDX-License-Identifier: MIT
/**
 * @file eloslog.h
 * @brief Header related to elos connection.
 */
#ifndef __ELOSLOG_H__
#define __ELOSLOG_H__

#include <stdbool.h>
#include <stdint.h>

#include "elos-common.h"

#define CRINIT_ELOSLOG_FEATURE_NAME "elos"
#define CRINIT_ELOSLOG_EVENT_LIMIT 0x400

/**
 * Initialize all components needed to handle event logging.
 */
int crinitEloslogInit(void);

/**
 * Specify if events should be sent to Elos.
 *
 * By default, Crinit will not connect to elos. If this is set to `true`, however, Crinit will connect
 * and send events to the elos daemon.
 *
 * Modifies errno.
 *
 * @param e  `true` if elos should be used, `false` otherwise.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitEloslogActivate(bool e);

/**
 * Log a crinit event to elos.
 *
 * Modifies errno.
 *
 * @param severity        The event severity.
 * @param messageCode     The event message code.
 * @param classification  The event classification bitmask.
 * @param format          Format string to format remaining arguments.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosLog(crinitElosSeverityE_t severity, crinitElosEventMessageCodeE_t messageCode, uint64_t classification,
                  const char *format, ...);

#endif /* __ELOSLOG_H__ */
