/**
 * @file sockcom.c
 * @brief Implementation of socket communication functions for the crinit-client library.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include "sockcom.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "logio.h"

/**
 * Wait for a ready-to-receive message from Crinit.
 *
 * @param sockFd  The socket file descriptor connected to Crinit.
 *
 * @return 0 on success, -1 otherwise
 */
static int waitForRTR(int sockFd);

int EBCL_crinitConnect(int *sockFd, const char *sockFile) {
    EBCL_dbgInfoPrint("Sending message to server at \'%s\'.", sockFile);

    *sockFd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (*sockFd == -1) {
        EBCL_errnoPrint("Could not create socket for connection to Crinit.");
        return -1;
    }
    struct sockaddr_un crinitServAddr;
    crinitServAddr.sun_family = AF_UNIX;
    strncpy(crinitServAddr.sun_path, sockFile, sizeof(crinitServAddr.sun_path) - 1);
    if (connect(*sockFd, (struct sockaddr *)&crinitServAddr, sizeof(struct sockaddr_un)) == -1) {
        EBCL_errnoPrint("Could not connect to Crinit through %s.", crinitServAddr.sun_path);
        close(*sockFd);
        return -1;
    }
    EBCL_dbgInfoPrint("Connected to Crinit.");
    EBCL_dbgInfoPrint("Waiting for RTR.");
    if (waitForRTR(*sockFd) == -1) {
        EBCL_errPrint("Could not wait for RTR.");
        close(*sockFd);
        return -1;
    }
    EBCL_dbgInfoPrint("Crinit is ready to receive.");
    return 0;
}

int EBCL_crinitSend(int sockFd, const ebcl_RtimCmd *cmd) {
    if (cmd == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL");
        return -1;
    }

    char *sendStr = NULL;
    size_t sendLen = 0;
    if (EBCL_rtimCmdToMsgStr(&sendStr, &sendLen, cmd) == -1) {
        EBCL_errPrint("Could not transform RtimCmd into sendable string.");
        return -1;
    }

    if (send(sockFd, &sendLen, sizeof(size_t), 0) == -1) {
        EBCL_errnoPrint("Could not send length packet (\'%lu\') of string \'%s\' to client.", sendLen, sendStr);
        free(sendStr);
        return -1;
    }

    if (send(sockFd, sendStr, sendLen, 0) == -1) {
        EBCL_errnoPrint("Could not send string \'%s\' to client.", sendStr);
        free(sendStr);
        return -1;
    }
    EBCL_dbgInfoPrint("Sent message of %d Bytes. Content:\n\'%s\'", sendLen, sendStr);
    free(sendStr);
    return 0;
}

int EBCL_crinitRecv(int sockFd, ebcl_RtimCmd *res) {
    if (res == NULL) {
        EBCL_errPrint("Return pointer must not be NULL.");
        return -1;
    }

    size_t recvLen = 0;
    ssize_t bytesRead = -1;
    bytesRead = recv(sockFd, &recvLen, sizeof(size_t), 0);
    if (bytesRead == -1) {
        EBCL_errnoPrint("Could not receive string length message via socket.");
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        EBCL_errPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    EBCL_dbgInfoPrint("Received message of %d Bytes. Content:\n\'%lu\'", bytesRead, recvLen);

    char *recvStr = malloc(recvLen);
    if (recvStr == NULL) {
        EBCL_errnoPrint("Could not allocate receive buffer of size %lu Bytes.", recvLen);
        return -1;
    }

    bytesRead = recv(sockFd, recvStr, recvLen, 0);
    if (bytesRead == -1) {
        free(recvStr);
        EBCL_errnoPrint("Could not receive string data message of size %lu Bytes via socket.", recvLen);
        return -1;
    }
    if (bytesRead != recvLen) {
        EBCL_errPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    EBCL_dbgInfoPrint("Received message of %d Bytes. Content:\n\'%s\'", bytesRead, recvStr);

    if (EBCL_parseRtimCmd(res, recvStr) == -1) {
        free(recvStr);
        EBCL_errPrint("Could not parse response message.");
        return -1;
    }
    free(recvStr);
    return 0;
}

static int waitForRTR(int sockFd) {
    char RtrBuf[sizeof("RTR")] = {'\0'};
    size_t recvLen = 0;
    ssize_t bytesRead = recv(sockFd, &recvLen, sizeof(size_t), 0);
    if (bytesRead == -1) {
        EBCL_errnoPrint("Could not receive string length message via socket.");
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        EBCL_errPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    EBCL_dbgInfoPrint("Received message of %d Bytes. Content:\n\'%lu\'", bytesRead, recvLen);
    if (recvLen != sizeof("RTR")) {
        EBCL_errPrint("Received unexpected string length for RTR: '%lu' Bytes", recvLen);
        return -1;
    }

    bytesRead = recv(sockFd, RtrBuf, recvLen, 0);
    if (bytesRead == -1) {
        EBCL_errnoPrint("Could not receive string data message of size %lu Bytes via socket.", recvLen);
        return -1;
    }
    if (bytesRead != recvLen) {
        EBCL_errPrint("Received data of unexpected length from Crinit: '%ld' Bytes", bytesRead);
        return -1;
    }
    RtrBuf[sizeof(RtrBuf) - 1] = '\0';
    EBCL_dbgInfoPrint("Received message of %d Bytes. Content:\n\'%s\'", bytesRead, RtrBuf);
    if (strncmp(RtrBuf, "RTR", strlen("RTR")) != 0) {
        EBCL_errPrint("Received \'%s\' rather than \'RTR\'.", RtrBuf);
        return -1;
    }
    return 0;
}
