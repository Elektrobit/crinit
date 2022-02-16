/**
 * @file sockcom.h
 * @brief Header defining Crinit socket communication functions used internally by the crinit-client library.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __SOCKCOM_H__
#define __SOCKCOM_H__

#include "rtimcmd.h"

/**
 * Perform a request/response transfer with Crinit
 *
 * Will connect to Crinit and send a request/command, then receive the result/response.
 * The server side equivalent is EBCL_connThread() in notiserv.c.
 *
 * The following image shows the high level communication sequence. For the lower level, refer to
 * the internal functions EBCL_crinitSend() and EBCL_crinitRecv().
 *
 * \image html notiserv_sock_comm_seq.svg
 *
 * @param sockFile  Path to the AF_UNIX socket file to connect to.
 * @param res       Return pointer for response/result.
 * @param cmd       The command/request to send.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_crinitXfer(const char *sockFile, ebcl_RtimCmd_t *res, const ebcl_RtimCmd_t *cmd);

#endif /* __SOCKCOM_H__ */
