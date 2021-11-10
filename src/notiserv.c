/**
 * @file notiserv.c
 * @brief Implementation of the notification and service interface.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#define _GNU_SOURCE  ///< Needed for SCM_CREDENTIALS, struct ucred,...
#include "notiserv.h"

#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <unistd.h>

#include "logio.h"
#include "rtimcmd.h"
#include "thrpool.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define gettid() ((pid_t)syscall(SYS_gettid))

/** Maximum number of unserviced connections until the server starts refusing **/
#define MAX_CONN_BACKLOG 100

/** Helper structure defining the arguments to connThread() **/
typedef struct ConnThrArgs {
    int sockFd;              ///< The socket to accept connections from.
    ebcl_ThreadPool *tpRef;  ///< Pointer to the ebcl_ThreadPool context, needed for
                             ///< EBCL_threadPoolThreadAvailCallback() and EBCL_threadPoolThreadBusyCallback().
} ConnThrArgs;

static ebcl_ThreadPool workers;  ///< The worker thread pool to run connThread() in.
static ebcl_TaskDB *tdbRef;      ///< Pointer to the ebcl_TaskDB to operate on.

/**
 * The worker thread function for handling a connection to a client.
 *
 * Will accept connections in a loop, send RTR, handle incoming requests, and send responses. Automatically gets
 * informed of client PID, UID, and GID through `SO_PASSCRED`/`SCM_CREDENTIALS`, so that permission handling is
 * possible.
 *
 * Uses EBCL_threadPoolThreadAvailCallback() and EBCL_threadPoolThreadBusyCallback() to signal its status to the thread
 * pool.
 *
 * The client-side equivalent connection-handling function is crinitXfer() in crinit-client.c. The following image
 * illustrates the high level client/server protocol.
 *
 * \image html notiserv_sock_comm_seq.svg
 *
 * @param args  Arguments to the function, see ConnThrArgs.
 *
 * @return  Does not return unless its thread is canceled in which case the return value is undefined.
 */
static void *connThread(void *args);
/**
 * Create AF_UNIX socket file, bind() and listen().
 *
 * Will overwrite any exist file at \a path.
 *
 * @param sockFd  Return pointer for the socket file descriptor.
 * @param path    Path to the socket file whcih should be created.
 *
 * @return 0 on success, -1 on error
 */
static int createSockFile(int *sockFd, const char *path);
/**
 * Checks if a received struct cmsghdr has expected length and contents.
 *
 * Expected is a struct cmsghdr containing SCM_CREDENTIALS.
 *
 * @param cmh  The header to check.
 *
 * @return   true if everything is as expected, false otherwise
 */
static inline bool cmsgHdrCheck(const struct cmsghdr *cmh);
/**
 * Checks if two struct ucred are equal.
 *
 * Two struct ucred are equal if and only if their members pid, uid, and gid are equal with their respective
 * counterparts.
 *
 * @param a  First operand.
 * @param b  Second operand.
 *
 * @return  true if \a a and \a b are equal, false if not
 */
static inline bool ucredCheckEqual(const struct ucred *a, const struct ucred *b);
/**
 * Sends a string to a connected client.
 *
 * The low level protocol is to first send a size_t informing the client of the length of the following string
 * (including the terminating zero) and then the string itself. The complementary client-side function is
 * EBCL_crinitSend().
 *
 * The following image illustrates the low level send/receive protocol:
 * \image html sock_comm_str.svg
 *
 * @param sockFd  The socket file descriptor connected to the client.
 * @param str     The string to send.
 *
 * @return 0 on success, -1 otherwise
 */
static inline int sendStr(int sockFd, const char *str);
/**
 * Receives a string from a connected client.
 *
 * The low level protocol is to first wait for a size_t informing the server of the length of the following string
 * (including the terminating zero) and then the string itself. The complementary client-side function is
 * EBCL_crinitRecv().
 *
 * This function will also extract the message metadata received via `SO_PASSCRED` and return the credentials of the
 * sender via \a passedCreds.
 *
 * This function will allocate memory for the received string using malloc(). The string should be free()'d when no
 * longer needed.
 *
 * The following image illustrates the low level send/receive protocol:
 * \image html sock_comm_str.svg
 *
 * @param sockFd       The socket file descriptor connected to the client.
 * @param str          Return pointer for the received string.
 * @param passedCreds  Return pointer for SCM_CREDENTIALS metadata.
 *
 * @return 0 on success, -1 otherwise
 */
static inline int recvStr(int sockFd, char **str, struct ucred *passedCreds);

/**
 * Recursive mkdir(), equivalent to `mkdir -p`.
 *
 * Note: Will modify the input \a pathname during execution to seperate paths.
 *
 * @param pathname  The complete path to recursively generate.
 * @param mode      The mode passed to mkdir().
 *
 * @return 0 on success, -1 on error
 */
static inline int mkdirp(char *pathname, mode_t mode);

int EBCL_startInterfaceServer(ebcl_TaskDB *ctx, const char *sockFile) {
    if (ctx == NULL || sockFile == NULL) {
        EBCL_errPrint("Given arguments must not be NULL.");
        return -1;
    }
    tdbRef = ctx;
    char *sockFileTmp = strdup(sockFile);
    if (sockFileTmp == NULL) {
        EBCL_errnoPrint("Could not duplicate string.");
        return -1;
    }

    char *sockDir = dirname(sockFileTmp);
    if (mkdirp(sockDir, 0777) == -1) {
        EBCL_errnoPrint("Could not create directory \'%s\'.", sockDir);
        free(sockFileTmp);
        return -1;
    }
    free(sockFileTmp);
    int sockFd = -1;
    umask(0);
    if (createSockFile(&sockFd, sockFile) == -1) {
        EBCL_errPrint("Could not create socket file at \'%s\'.", sockFile);
        return -1;
    }
    umask(0022);
    ConnThrArgs a = {sockFd, &workers};
    if (EBCL_threadPoolInit(&workers, 0, connThread, &a, sizeof(ConnThrArgs)) == -1) {
        EBCL_errPrint("Could not fill server thread pool.");
        return -1;
    }

    return 0;
}

static inline int mkdirp(char *pathName, mode_t mode) {
    if (pathName == NULL) {
        EBCL_errPrint("Input path name must not be NULL");
        return -1;
    }

    char *runner = pathName;
    while (*runner != '\0' && *runner == '/') {
        runner++;
    }

    // Run mkdir for every slash-delimited substring of the path from front to back, ignoring already existing parts.
    while (runner != NULL) {
        runner = strchr(runner, '/');
        if (runner != NULL) {
            *runner = '\0';
        }
        if (mkdir(pathName, mode) == -1 && errno != EEXIST) {
            EBCL_errnoPrint("Could not create directory \'%s\'.", pathName);
            return -1;
        }
        if (runner != NULL) {
            *runner = '/';
            runner++;
        }
    }
    return 0;
}

static inline bool cmsgHdrCheck(const struct cmsghdr *cmh) {
    if (cmh == NULL) {
        return false;
    }
    if (cmh->cmsg_len != CMSG_LEN(sizeof(struct ucred))) {
        return false;
    }
    if (cmh->cmsg_level != SOL_SOCKET) {
        return false;
    }
    if (cmh->cmsg_type != SCM_CREDENTIALS) {
        return false;
    }
    return true;
}

static void *connThread(void *args) {
    pid_t threadId = gettid();
    if (args == NULL) {
        EBCL_errPrint("(TID %d) Argument to connection worker thread must not be NULL.", threadId);
        return NULL;
    }
    ConnThrArgs *a = (ConnThrArgs *)args;
    int servSockFd = a->sockFd;
    int connSockFd;
    EBCL_dbgInfoPrint("(TID %d) Connection worker thread ready.", threadId);
    while (true) {
        connSockFd = -1;
        connSockFd = accept(servSockFd, NULL, NULL);
        EBCL_threadPoolThreadBusyCallback(a->tpRef);

        if (connSockFd == -1) {
            EBCL_errnoPrint("(TID %d) Could not accept connection.", threadId);
            continue;
        }

        int optVal = 1;
        if (setsockopt(connSockFd, SOL_SOCKET, SO_PASSCRED, &optVal, sizeof(int)) == -1) {
            EBCL_errnoPrint("(TID %d) Could not set SO_PASSCRED option for connection socket.", threadId);
            close(connSockFd);
            continue;
        }
        if (sendStr(connSockFd, "RTR") == -1) {
            EBCL_errPrint("(TID %d) Could not send RTR-message to client.", threadId);
            close(connSockFd);
            continue;
        }
        struct ucred msgCreds = {0};
        char *clientMsg = NULL;
        if (recvStr(connSockFd, &clientMsg, &msgCreds) == -1) {
            EBCL_errPrint("(TID %d) Could not receive string message from client.", threadId);
            close(connSockFd);
            continue;
        }

        EBCL_dbgInfoPrint("(TID %d) Received string \'%s\' from client.", threadId, clientMsg);
        EBCL_dbgInfoPrint("(TID %d) Received following credentials from peer process: PID=%d, UID=%d, GID=%d", threadId,
                          msgCreds.pid, msgCreds.uid, msgCreds.gid);

        ebcl_RtimCmd cmd, res;
        if (EBCL_parseRtimCmd(&cmd, clientMsg) == -1) {
            EBCL_errPrint("(TID %d) Could not parse command from client.", threadId);
            free(clientMsg);
            close(connSockFd);
            continue;
        }
        free(clientMsg);
        if (cmd.op != EBCL_RTIMCMD_C_STATUS && msgCreds.uid != 0) {
            EBCL_errPrint("(TID %d) Client does not have permission to issue command.", threadId);
            if (EBCL_buildRtimCmd(&res, cmd.op + 1, 2, EBCL_RTIMCMD_RES_ERR, "Permission denied.") == -1) {
                EBCL_errPrint("Could not generate response to client.");
                EBCL_destroyRtimCmd(&cmd);
                close(connSockFd);
                continue;
            }
        } else {
            if (EBCL_execRtimCmd(tdbRef, &res, &cmd) == -1) {
                EBCL_errPrint("(TID %d) Could not execute command from client.", threadId);
                EBCL_destroyRtimCmd(&cmd);
                close(connSockFd);
                continue;
            }
        }
        EBCL_destroyRtimCmd(&cmd);
        char *resStr;
        size_t resLen;
        if (EBCL_rtimCmdToMsgStr(&resStr, &resLen, &res) == -1) {
            EBCL_destroyRtimCmd(&res);
            EBCL_errPrint("(TID %d) Could not transform command result to response string.", threadId);
            continue;
        }
        EBCL_destroyRtimCmd(&res);
        EBCL_dbgInfoPrint("(TID %d) Will send response message \'%s\' to client.", threadId, resStr);
        if (sendStr(connSockFd, resStr) == -1) {
            EBCL_errPrint("(TID %d) Could not send response message to client.", threadId);
            close(connSockFd);
            continue;
        }

        close(connSockFd);
        EBCL_threadPoolThreadAvailCallback(a->tpRef);
    }
    return NULL;
}

static int createSockFile(int *sockFd, const char *path) {
    if (sockFd == NULL) {
        EBCL_errPrint("Return pointer for socket file descriptor must not be NULL.");
        return -1;
    }
    *sockFd = -1;
    struct sockaddr_un servAddr;

    if ((*sockFd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) == -1) {
        EBCL_errnoPrint("Could not create server socket.");
        return -1;
    }

    memset(&servAddr, 0, sizeof(struct sockaddr_un));
    servAddr.sun_family = AF_UNIX;
    strncpy(servAddr.sun_path, path, sizeof(servAddr.sun_path) - 1);

    unlink(path);
    if (bind(*sockFd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr_un)) == -1) {
        EBCL_errnoPrint("Could not bind to server socket.");
        return -1;
    }

    if (listen(*sockFd, MAX_CONN_BACKLOG) == -1) {
        EBCL_errnoPrint("Error trying to set server socket as listening.");
        return -1;
    }
    return 0;
}

static inline int sendStr(int sockFd, const char *str) {
    pid_t threadId = gettid();
    if (str == NULL) {
        EBCL_errPrint("(TID %d) String to send must not be NULL.", threadId);
        return -1;
    }

    size_t dataLen = strlen(str) + 1;
    if (send(sockFd, &dataLen, sizeof(size_t), 0) == -1) {
        EBCL_errnoPrint("(TID %d) Could not send length packet (\'%lu\') of string \'%s\' to client. %d", threadId,
                        dataLen, str, sockFd);
        return -1;
    }

    if (send(sockFd, str, dataLen, 0) == -1) {
        EBCL_errnoPrint("(TID %d) Could not send string \'%s\' to client.", threadId, str);
        return -1;
    }

    return 0;
}

static inline int recvStr(int sockFd, char **str, struct ucred *passedCreds) {
    pid_t threadId = gettid();
    if (str == NULL || passedCreds == NULL) {
        EBCL_errPrint("(TID %d) Pointer arguments must not be NULL.", threadId);
        return -1;
    }

    union {
        char alignedBuf[CMSG_SPACE(sizeof(struct ucred))];
        struct cmsghdr alignment;
    } ancillaryData;
    struct ucred scmCreds[2] = {0};

    struct iovec iov;
    struct msghdr mHdr;
    memset(&mHdr, 0, sizeof(struct msghdr));
    mHdr.msg_name = NULL;
    mHdr.msg_namelen = 0;
    mHdr.msg_iov = &iov;
    mHdr.msg_iovlen = 1;

    size_t dataLen = 0;
    iov.iov_base = &dataLen;
    iov.iov_len = sizeof(size_t);

    mHdr.msg_control = ancillaryData.alignedBuf;
    mHdr.msg_controllen = sizeof(ancillaryData.alignedBuf);
    ssize_t bytesRead = -1;
    bytesRead = recvmsg(sockFd, &mHdr, 0);
    if (bytesRead == -1) {
        EBCL_errnoPrint("(TID %d) Could not receive string length message via socket.", threadId);
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        EBCL_errPrint("Received data of unexpected length from client: %ld Bytes", bytesRead);
        return -1;
    }
    EBCL_dbgInfoPrint("(TID %d) Received message of %d Bytes. Content:\n\'%lu\'", threadId, bytesRead, dataLen);

    struct cmsghdr *cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!cmsgHdrCheck(cmHdr)) {
        EBCL_errPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        return -1;
    }
    memcpy(&scmCreds[0], CMSG_DATA(cmHdr), sizeof(struct ucred));

    *str = malloc(dataLen);
    if (*str == NULL) {
        EBCL_errnoPrint("(TID %d) Could not allocate receive buffer of size %lu Bytes.", threadId, dataLen);
        return -1;
    }
    iov.iov_base = *str;
    iov.iov_len = dataLen;

    bytesRead = recvmsg(sockFd, &mHdr, 0);
    if (bytesRead == -1) {
        EBCL_errnoPrint("(TID %d) Could not receive string data message of size %lu Bytes via socket.", threadId,
                        dataLen);
        goto fail;
    }
    if (bytesRead != dataLen) {
        EBCL_errPrint("Received data of unexpected length from client: %ld Bytes vs. announced %lu Bytes ", bytesRead,
                      dataLen);
        goto fail;
    }
    EBCL_dbgInfoPrint("(TID %d) Received message of %d Bytes. Content:\n\'%s\'", threadId, bytesRead, *str);

    cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!cmsgHdrCheck(cmHdr)) {
        EBCL_errPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        goto fail;
    }
    memcpy(&scmCreds[1], CMSG_DATA(cmHdr), sizeof(struct ucred));

    if (!ucredCheckEqual(&scmCreds[0], &scmCreds[1])) {
        EBCL_errPrint(
            "(TID %d) Ancillary credential data of the length and data parts of the string message do not match.",
            threadId);
        goto fail;
    }

    memcpy(passedCreds, scmCreds, sizeof(struct ucred));
    return 0;
fail:
    free(*str);
    *str = NULL;
    return -1;
}

static inline bool ucredCheckEqual(const struct ucred *a, const struct ucred *b) {
    return (a->pid == b->pid) && (a->uid == b->uid) && (a->gid == b->gid);
}

