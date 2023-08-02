// SPDX-License-Identifier: MIT
/**
 * @file sockcom.h
 * @brief Header defining Crinit socket communication functions used internally by the crinit-client library.
 */
#ifndef __SOCKCOM_H__
#define __SOCKCOM_H__

#include "rtimcmd.h"

/**
 * Perform a request/response transfer with Crinit
 *
 * Will connect to Crinit and send a request/command, then receive the result/response.
 * The server side equivalent is crinitConnThread() in notiserv.c.
 *
 * The following image shows the high level communication sequence. For the lower level, refer to
 * the internal functions crinitSend() and crinitRecv().
 *
 * \image html notiserv_sock_comm_seq.svg
 *
 * @param sockFile  Path to the AF_UNIX socket file to connect to.
 * @param res       Return pointer for response/result.
 * @param cmd       The command/request to send.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitXfer(const char *sockFile, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd);

#endif /* __SOCKCOM_H__ */
