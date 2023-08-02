// SPDX-License-Identifier: MIT
/**
 * @file notiserv.h
 * @brief Header related to the notification and service interface.
 */
#ifndef __NOTISERV_H__
#define __NOTISERV_H__

#include "taskdb.h"

/**
 * Starts the Notification and Service interface socket server.
 *
 * Will create the AF_UNIX socket for clients to connect to and spawn a number of worker threads to service incoming
 * connections (see notiserv.c and thrpool.h).
 *
 * @param ctx       Pointer to the crinitTaskDB_t which the Server should use for incoming requests.
 * @param sockfile  Path where to create the AF_UNIX socket file.
 *
 * @return 0 on success, -1 on error
 */
int crinitStartInterfaceServer(crinitTaskDB_t *ctx, const char *sockfile);

#endif /*__NOTISERV_H__ */
