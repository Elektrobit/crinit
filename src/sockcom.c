// SPDX-License-Identifier: MIT
/**
 * @file sockcom.c
 * @brief Implementation of socket communication functions for the crinit-client library.
 */
#include "sockcom.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "logio.h"

/**
 * Connect to Crinit and wait for a ready-to-receive message.
 *
 * @param sockFd    Return pointer for the connected socket.
 * @param sockFile  Path to the AF_UNIX socket file to connect to.
 *
 * @return 0 on success, -1 otherwise
 */
static int crinitConnect(int *sockFd, const char *sockFile);
/**
 * Send a command/request to Crinit.
 *
 * Uses crinitRtimCmdToMsgStr() to generate a string and sends it using the same protocol as sendStr()/recvStr() in
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
static int crinitSend(int sockFd, const crinitRtimCmd_t *cmd);
/**
 * Receive a response from Crinit.
 *
 * Receives a string using the same protocol as sendStr()/recvStr() in notiserv.c and then uses crinitParseRtimCmd() to
 * generate an equivalent crinitRtimCmd_t.
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
static int crinitRecv(int sockFd, crinitRtimCmd_t *res);
/**
 * Wait for a ready-to-receive message from Crinit.
 *
 * @param sockFd  The socket file descriptor connected to Crinit.
 *
 * @return 0 on success, -1 otherwise
 */
static int crinitWaitForRtr(int sockFd);

int crinitXfer(const char *sockFile, crinitRtimCmd_t *res, const crinitRtimCmd_t *cmd) {
    if (res == NULL || cmd == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL");
        return -1;
    }
    int sockFd = -1;
    if (crinitConnect(&sockFd, sockFile) == -1) {
        crinitErrPrint("Could not connect to Crinit using socket at \'%s\'.", sockFile);
        return -1;
    }
    crinitDbgInfoPrint("Connected to Crinit using %s.", sockFile);
    if (crinitSend(sockFd, cmd) == -1) {
        crinitErrPrint("Could not send RtimCmd to Crinit.");
        return -1;
    }
    if (crinitRecv(sockFd, res) == -1) {
        crinitErrPrint("Could not receive response from Crinit.");
        return -1;
    }
    close(sockFd);
    return 0;
}

static int crinitConnect(int *sockFd, const char *sockFile) {
    crinitDbgInfoPrint("Sending message to server at \'%s\'.", sockFile);

    *sockFd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (*sockFd == -1) {
        crinitErrnoPrint("Could not create socket for connection to Crinit.");
        return -1;
    }
    struct sockaddr_un crinitServAddr;
    crinitServAddr.sun_family = AF_UNIX;
    size_t sockPathLen = strnlen(sockFile, sizeof(crinitServAddr.sun_path));
    if (sockPathLen == sizeof(crinitServAddr.sun_path)) {
        crinitErrPrint("Path to socket file is longer than %zu characters.", sizeof(crinitServAddr.sun_path) - 1);
        close(*sockFd);
        return -1;
    }
    strcpy(crinitServAddr.sun_path, sockFile);

    if (connect(*sockFd, (struct sockaddr *)&crinitServAddr, sizeof(struct sockaddr_un)) == -1) {
        crinitErrnoPrint("Could not connect to Crinit through %s.", crinitServAddr.sun_path);
        close(*sockFd);
        return -1;
    }
    crinitDbgInfoPrint("Connected to Crinit.");
    crinitDbgInfoPrint("Waiting for RTR.");
    if (crinitWaitForRtr(*sockFd) == -1) {
        crinitErrPrint("Could not wait for RTR.");
        close(*sockFd);
        return -1;
    }
    crinitDbgInfoPrint("Crinit is ready to receive.");
    return 0;
}

static int crinitSend(int sockFd, const crinitRtimCmd_t *cmd) {
    if (cmd == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL");
        return -1;
    }

    char *sendStr = NULL;
    size_t sendLen = 0;
    if (crinitRtimCmdToMsgStr(&sendStr, &sendLen, cmd) == -1) {
        crinitErrPrint("Could not transform RtimCmd into sendable string.");
        return -1;
    }

    if (send(sockFd, &sendLen, sizeof(size_t), MSG_NOSIGNAL) == -1) {
        crinitErrnoPrint("Could not send length packet (\'%zu\') of string \'%s\' to client.", sendLen, sendStr);
        free(sendStr);
        return -1;
    }

    if (send(sockFd, sendStr, sendLen, MSG_NOSIGNAL) == -1) {
        crinitErrnoPrint("Could not send string \'%s\' to client.", sendStr);
        free(sendStr);
        return -1;
    }
    crinitDbgInfoPrint("Sent message of %zu Bytes. Content:\n\'%s\'", sendLen, sendStr);
    free(sendStr);
    return 0;
}

static int crinitRecv(int sockFd, crinitRtimCmd_t *res) {
    if (res == NULL) {
        crinitErrPrint("Return pointer must not be NULL.");
        return -1;
    }

    size_t recvLen = 0;
    ssize_t bytesRead = -1;
    bytesRead = recv(sockFd, &recvLen, sizeof(size_t), 0);
    if (bytesRead < 0) {
        crinitErrnoPrint("Could not receive string length message via socket.");
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        crinitErrPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    crinitDbgInfoPrint("Received message of %ld Bytes. Content:\n\'%zu\'", bytesRead, recvLen);

    char *recvStr = malloc(recvLen);
    if (recvStr == NULL) {
        crinitErrnoPrint("Could not allocate receive buffer of size %zu Bytes.", recvLen);
        return -1;
    }

    bytesRead = recv(sockFd, recvStr, recvLen, 0);
    if (bytesRead < 0) {
        free(recvStr);
        crinitErrnoPrint("Could not receive string data message of size %zu Bytes via socket.", recvLen);
        return -1;
    }
    if ((size_t)bytesRead != recvLen) {
        crinitErrPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    // force terminating zero
    recvStr[recvLen - 1] = '\0';
    crinitDbgInfoPrint("Received message of %ld Bytes. Content:\n\'%s\'", bytesRead, recvStr);

    if (crinitParseRtimCmd(res, recvStr) == -1) {
        free(recvStr);
        crinitErrPrint("Could not parse response message.");
        return -1;
    }
    free(recvStr);
    return 0;
}

static int crinitWaitForRtr(int sockFd) {
    char rtrBuf[sizeof("RTR")] = {'\0'};
    size_t recvLen = 0;
    ssize_t bytesRead = recv(sockFd, &recvLen, sizeof(size_t), 0);
    if (bytesRead < 0) {
        crinitErrnoPrint("Could not receive string length message via socket.");
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        crinitErrPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    crinitDbgInfoPrint("Received message of %ld Bytes. Content:\n\'%zu\'", bytesRead, recvLen);
    if (recvLen != sizeof("RTR")) {
        crinitErrPrint("Received unexpected string length for RTR: '%zu' Bytes", recvLen);
        return -1;
    }

    bytesRead = recv(sockFd, rtrBuf, recvLen, 0);
    if (bytesRead < 0) {
        crinitErrnoPrint("Could not receive string data message of size %zu Bytes via socket.", recvLen);
        return -1;
    }
    if ((size_t)bytesRead != recvLen) {
        crinitErrPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    rtrBuf[sizeof(rtrBuf) - 1] = '\0';
    crinitDbgInfoPrint("Received message of %ld Bytes. Content:\n\'%s\'", bytesRead, rtrBuf);
    if (strncmp(rtrBuf, "RTR", strlen("RTR")) != 0) {
        crinitErrPrint("Received \'%s\' rather than \'RTR\'.", rtrBuf);
        return -1;
    }
    return 0;
}
