/**
 * @file notiserv.h
 * @brief Header related to the notification and service interface.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __NOTISERV_H__
#define __NOTISERV_H__

#include "taskdb.h"

/**
 * The standard path of the socket file to create/bind to for the Notification/Service Interface.
 */
#define EBCL_CRINIT_SOCKFILE "/run/crinit/crinit.sock"

/**
 * Starts the Notification and Service interface socket server.
 *
 * Will create the AF_UNIX socket for clients to connect to and spawn a number of worker threads to service incoming
 * connections (see notiserv.c and thrpool.h).
 *
 * @param ctx       Pointer to the ebcl_TaskDB which the Server should use for incoming requests.
 * @param sockfile  Path where to create the AF_UNIX socket file.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_startInterfaceServer(ebcl_TaskDB *ctx, const char *sockfile);

#endif /*__NOTISERV_H__ */
