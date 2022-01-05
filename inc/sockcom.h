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
 * Connect to Crinit and wait for a ready-to-receive message.
 *
 * @param sockFd    Return pointer for the connected socket.
 * @param sockFile  Path to the AF_UNIX socket file to connect to.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_crinitConnect(int *sockFd, const char *sockFile);
/**
 * Send a command/request to Crinit.
 *
 * Uses EBCL_rtimCmdToMsgStr() to generate a string and sends it using the same protocol as sendStr()/recvStr() in
 * notiserv.c. First, a binary size_t with the string size is sent, then the string itself in a second message/packet.
 *
 * The following diagram illustrates the low-level protocol:
 * \image html sock_comm_str.svg
 *
 * @param sockFd  The connected socket over which to send.
 * @param cmd     The command/request to send.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_crinitSend(int sockFd, const ebcl_RtimCmd_t *cmd);
/**
 * Receive a response from Crinit.
 *
 * Receives a string using the same protocol as sendStr()/recvStr() in notiserv.c and then uses EBCL_parseRtimCmd() to
 * generate an equivalent ebcl_RtimCmd_t.
 *
 * First, a binary size_t with the string size is received, memory allocation made accordingly, and then the string
 * itself in a second message/packet is received.
 *
 * The following diagram illustrates the low-level protocol:
 * \image html sock_comm_str.svg
 *
 * @param sockFd  The connected socket from which to receive.
 * @param res     Return pointer for the response/result.
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_crinitRecv(int sockFd, ebcl_RtimCmd_t *res);

#endif /* __SOCKCOM_H__ */
