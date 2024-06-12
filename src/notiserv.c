// SPDX-License-Identifier: MIT
/**
 * @file notiserv.c
 * @brief Implementation of the notification and service interface.
 */
#define _GNU_SOURCE  ///< Needed for SCM_CREDENTIALS, struct ucred,...
#include "notiserv.h"

#include <libgen.h>
#include <linux/capability.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <unistd.h>

#include "eloslog.h"
#include "logio.h"
#include "rtimcmd.h"
#include "thrpool.h"

#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/** Macro wrapper for the gettid syscall in case glibc is not new enough to contain one itself **/
#define crinitGettid() ((pid_t)syscall(SYS_gettid))

/** Maximum number of unserviced connections until the server starts refusing **/
#define MAX_CONN_BACKLOG 100

/** Helper structure defining the arguments to connThread() **/
typedef struct crinitConnThrArgs_t {
    int sockFd;                 ///< The socket to accept connections from.
    crinitThreadPool_t *tpRef;  ///< Pointer to the crinitThreadPool_t context, needed for
                                ///< crinitThreadPoolThreadAvailCallback() and crinitThreadPoolThreadBusyCallback().
} crinitConnThrArgs_t;

static crinitThreadPool_t crinitWorkers;  ///< The worker thread pool to run connThread() in.
static crinitTaskDB_t *crinitTdbRef;      ///< Pointer to the crinitTaskDB_t to operate on.

/**
 * The worker thread function for handling a connection to a client.
 *
 * Will accept connections in a loop, send RTR, handle incoming requests, and send responses. Automatically gets
 * informed of client PID, UID, and GID through `SO_PASSCRED`/`SCM_CREDENTIALS`, so that permission handling is
 * possible.
 *
 * Uses crinitThreadPoolThreadAvailCallback() and crinitThreadPoolThreadBusyCallback() to signal its status to the
 * thread pool.
 *
 * The client-side equivalent connection-handling function is crinitXfer() in crinit-client.c. The following image
 * illustrates the high level client/server protocol.
 *
 * \image html notiserv_sock_comm_seq.svg
 *
 * @param args  Arguments to the function, see crinitConnThrArgs_t.
 *
 * @return  Does not return unless its thread is canceled in which case the return value is undefined.
 */
static void *crinitConnThread(void *args);
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
static int crinitCreateSockFile(int *sockFd, const char *path);
/**
 * Checks if a received struct cmsghdr has expected length and contents.
 *
 * Expected is a struct cmsghdr containing SCM_CREDENTIALS.
 *
 * @param cmh  The header to check.
 *
 * @return   true if everything is as expected, false otherwise
 */
static inline bool crinitCmsgHdrCheck(const struct cmsghdr *cmh);
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
static inline bool crinitUcredCheckEqual(const struct ucred *a, const struct ucred *b);
/**
 * Sends a string to a connected client.
 *
 * The low level protocol is to first send a size_t informing the client of the length of the following string
 * (including the terminating zero) and then the string itself. The complementary client-side function is
 * crinitSend().
 *
 * The following image illustrates the low level send/receive protocol:
 * \image html sock_comm_str.svg
 *
 * @param sockFd  The socket file descriptor connected to the client.
 * @param str     The string to send.
 *
 * @return 0 on success, -1 otherwise
 */
static inline int crinitSendStr(int sockFd, const char *str);
/**
 * Receives a string from a connected client.
 *
 * The low level protocol is to first wait for a size_t informing the server of the length of the following string
 * (including the terminating zero) and then the string itself. The complementary client-side function is
 * crinitRecv().
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
static inline int crinitRecvStr(int sockFd, char **str, struct ucred *passedCreds);
/**
 * Checks if given process credentials imply permission to execute remote command.
 *
 * @param op           The opcode of the command the process requests to execute.
 * @param passedCreds  The credentials of the requesting process obtained via SCM_CREDENTIALS.
 *
 * @return true if \a passedCreds imply the sending process is permitted to execute \a op. Returns false otherwise, also
 * on errors.
 */
static inline bool crinitCheckPerm(crinitRtimOp_t op, const struct ucred *passedCreds);
/**
 * Gets capabilities of process specified by PID.
 *
 * @param out  Return pointer for capabilities. Note, that the Linux API defines cap_user_data_t as a pointer to
 *             struct __user_cap_header_struct. The given pointer needs to point to at least two elements.
 * @param pid  PID of the process from which to get the capabilities.
 *
 * @return 0 on success, -1 on error
 */
static inline int crinitProcCapget(cap_user_data_t out, pid_t pid);

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
static inline int crinitMkdirp(char *pathname, mode_t mode);

int crinitStartInterfaceServer(crinitTaskDB_t *ctx, const char *sockFile) {
    if (ctx == NULL || sockFile == NULL) {
        crinitErrPrint("Given arguments must not be NULL.");
        return -1;
    }

    crinitTdbRef = ctx;
    char *sockFileTmp = strdup(sockFile);
    if (sockFileTmp == NULL) {
        crinitErrnoPrint("Could not duplicate string.");
        return -1;
    }

    char *sockDir = dirname(sockFileTmp);
    if (crinitMkdirp(sockDir, 0777) == -1) {
        crinitErrnoPrint("Could not create directory \'%s\'.", sockDir);
        free(sockFileTmp);
        return -1;
    }
    free(sockFileTmp);
    int sockFd = -1;
    umask(0);
    if (crinitCreateSockFile(&sockFd, sockFile) == -1) {
        crinitErrPrint("Could not create socket file at \'%s\'.", sockFile);
        return -1;
    }
    umask(0022);
    crinitConnThrArgs_t a = {sockFd, &crinitWorkers};
    if (crinitThreadPoolInit(&crinitWorkers, 0, crinitConnThread, &a, sizeof(crinitConnThrArgs_t)) == -1) {
        crinitErrPrint("Could not fill server thread pool.");
        return -1;
    }

    return 0;
}

static inline int crinitMkdirp(char *pathName, mode_t mode) {
    if (pathName == NULL) {
        crinitErrPrint("Input path name must not be NULL");
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
            crinitErrnoPrint("Could not create directory \'%s\'.", pathName);
            return -1;
        }
        if (runner != NULL) {
            *runner = '/';
            runner++;
        }
    }
    return 0;
}

static inline bool crinitCmsgHdrCheck(const struct cmsghdr *cmh) {
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

static void *crinitConnThread(void *args) {
    pid_t threadId = crinitGettid();
    if (args == NULL) {
        crinitErrPrint("(TID %d) Argument to connection worker thread must not be NULL.", threadId);
        return NULL;
    }
    crinitConnThrArgs_t *a = (crinitConnThrArgs_t *)args;
    int servSockFd = a->sockFd;
    int connSockFd;
    crinitDbgInfoPrint("(TID %d) Connection worker thread ready.", threadId);
    while (true) {
        connSockFd = -1;
        connSockFd = accept(servSockFd, NULL, NULL);
        crinitThreadPoolThreadBusyCallback(a->tpRef);

        if (connSockFd == -1) {
            crinitErrnoPrint("(TID %d) Could not accept connection.", threadId);
            continue;
        }

        int optVal = 1;
        if (setsockopt(connSockFd, SOL_SOCKET, SO_PASSCRED, &optVal, sizeof(int)) == -1) {
            crinitErrnoPrint("(TID %d) Could not set SO_PASSCRED option for connection socket.", threadId);
            close(connSockFd);
            continue;
        }
        if (crinitSendStr(connSockFd, "RTR") == -1) {
            crinitErrPrint("(TID %d) Could not send RTR-message to client.", threadId);
            close(connSockFd);
            continue;
        }
        struct ucred msgCreds = {0};
        char *clientMsg = NULL;
        if (crinitRecvStr(connSockFd, &clientMsg, &msgCreds) == -1) {
            crinitErrPrint("(TID %d) Could not receive string message from client.", threadId);
            close(connSockFd);
            continue;
        }

        crinitDbgInfoPrint("(TID %d) Received string \'%s\' from client.", threadId, clientMsg);
        crinitDbgInfoPrint("(TID %d) Received following credentials from peer process: PID=%d, UID=%d, GID=%d",
                           threadId, msgCreds.pid, msgCreds.uid, msgCreds.gid);

        crinitRtimCmd_t cmd, res;
        if (crinitParseRtimCmd(&cmd, clientMsg) == -1) {
            crinitErrPrint("(TID %d) Could not parse command from client.", threadId);
            free(clientMsg);
            close(connSockFd);
            continue;
        }
        free(clientMsg);
        if (!crinitCheckPerm(cmd.op, &msgCreds)) {
            crinitErrPrint("(TID %d) Client does not have permission to issue command.", threadId);
            if (crinitElosLog(ELOS_SEVERITY_WARN, ELOS_MSG_CODE_IPC_NOT_AUTHORIZED, "%d", msgCreds.pid) == -1) {
                crinitErrPrint("Could not enqueue elos permission event. Will continue but logging may be impaired.");
            }
            if (crinitBuildRtimCmd(&res, cmd.op + 1, 2, CRINIT_RTIMCMD_RES_ERR, "Permission denied.") == -1) {
                crinitErrPrint("Could not generate response to client.");
                crinitDestroyRtimCmd(&cmd);
                close(connSockFd);
                continue;
            }
        } else {
            if (crinitExecRtimCmd(crinitTdbRef, &res, &cmd) == -1) {
                crinitErrPrint("(TID %d) Could not execute command from client.", threadId);
                crinitDestroyRtimCmd(&cmd);
                close(connSockFd);
                continue;
            }
        }
        crinitDestroyRtimCmd(&cmd);
        char *resStr;
        size_t resLen;
        if (crinitRtimCmdToMsgStr(&resStr, &resLen, &res) == -1) {
            crinitDestroyRtimCmd(&res);
            crinitErrPrint("(TID %d) Could not transform command result to response string.", threadId);
            continue;
        }
        crinitDestroyRtimCmd(&res);
        crinitDbgInfoPrint("(TID %d) Will send response message \'%s\' to client.", threadId, resStr);
        if (crinitSendStr(connSockFd, resStr) == -1) {
            crinitErrPrint("(TID %d) Could not send response message to client.", threadId);
            free(resStr);
            close(connSockFd);
            continue;
        }

        free(resStr);
        close(connSockFd);
        crinitThreadPoolThreadAvailCallback(a->tpRef);
    }
    return NULL;
}

static int crinitCreateSockFile(int *sockFd, const char *path) {
    if (sockFd == NULL) {
        crinitErrPrint("Return pointer for socket file descriptor must not be NULL.");
        return -1;
    }
    *sockFd = -1;
    struct sockaddr_un servAddr;

    if ((*sockFd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) == -1) {
        crinitErrnoPrint("Could not create server socket.");
        return -1;
    }

    memset(&servAddr, 0, sizeof(struct sockaddr_un));
    servAddr.sun_family = AF_UNIX;
    strncpy(servAddr.sun_path, path, sizeof(servAddr.sun_path) - 1);

    if (bind(*sockFd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr_un)) == -1) {
        crinitErrnoPrint("Could not bind to server socket.");
        return -1;
    }

    if (listen(*sockFd, MAX_CONN_BACKLOG) == -1) {
        crinitErrnoPrint("Error trying to set server socket as listening.");
        return -1;
    }
    return 0;
}

static inline int crinitSendStr(int sockFd, const char *str) {
    pid_t threadId = crinitGettid();
    if (str == NULL) {
        crinitErrPrint("(TID %d) String to send must not be NULL.", threadId);
        return -1;
    }

    size_t dataLen = strlen(str) + 1;
    if (send(sockFd, &dataLen, sizeof(size_t), MSG_NOSIGNAL) == -1) {
        crinitErrnoPrint("(TID %d) Could not send length packet (\'%zu\') of string \'%s\' to client. %d", threadId,
                         dataLen, str, sockFd);
        return -1;
    }

    if (send(sockFd, str, dataLen, MSG_NOSIGNAL) == -1) {
        crinitErrnoPrint("(TID %d) Could not send string \'%s\' to client.", threadId, str);
        return -1;
    }

    return 0;
}

static inline int crinitRecvStr(int sockFd, char **str, struct ucred *passedCreds) {
    pid_t threadId = crinitGettid();
    if (str == NULL || passedCreds == NULL) {
        crinitErrPrint("(TID %d) Pointer arguments must not be NULL.", threadId);
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
        crinitErrnoPrint("(TID %d) Could not receive string length message via socket.", threadId);
        return -1;
    }
    if (bytesRead != sizeof(size_t)) {
        crinitErrPrint("Received data of unexpected length from client: %ld Bytes", bytesRead);
        return -1;
    }
    crinitDbgInfoPrint("(TID %d) Received message of %ld Bytes. Content:\n\'%zu\'", threadId, bytesRead, dataLen);

    struct cmsghdr *cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!crinitCmsgHdrCheck(cmHdr)) {
        crinitErrPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        return -1;
    }
    memcpy(&scmCreds[0], CMSG_DATA(cmHdr), sizeof(struct ucred));

    *str = malloc(dataLen);
    if (*str == NULL) {
        crinitErrnoPrint("(TID %d) Could not allocate receive buffer of size %zu Bytes.", threadId, dataLen);
        return -1;
    }
    iov.iov_base = *str;
    iov.iov_len = dataLen;

    bytesRead = recvmsg(sockFd, &mHdr, 0);
    if (bytesRead < 0) {
        crinitErrnoPrint("(TID %d) Could not receive string data message of size %zu Bytes via socket.", threadId,
                         dataLen);
        goto fail;
    }
    if ((size_t)bytesRead != dataLen) {
        crinitErrPrint("Received data of unexpected length from client: %ld Bytes vs. announced %zu Bytes ", bytesRead,
                       dataLen);
        goto fail;
    }
    // force terminating zero
    (*str)[dataLen - 1] = '\0';
    crinitDbgInfoPrint("(TID %d) Received message of %ld Bytes. Content:\n\'%s\'", threadId, bytesRead, *str);

    cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!crinitCmsgHdrCheck(cmHdr)) {
        crinitErrPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        goto fail;
    }
    memcpy(&scmCreds[1], CMSG_DATA(cmHdr), sizeof(struct ucred));

    if (!crinitUcredCheckEqual(&scmCreds[0], &scmCreds[1])) {
        crinitErrPrint(
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

static inline bool crinitUcredCheckEqual(const struct ucred *a, const struct ucred *b) {
    return (a->pid == b->pid) && (a->uid == b->uid) && (a->gid == b->gid);
}

static inline bool crinitCheckPerm(crinitRtimOp_t op, const struct ucred *passedCreds) {
    if (passedCreds == NULL) {
        crinitErrPrint("Pointer arguments must not be NULL.");
        return false;
    }

    struct __user_cap_data_struct capdata[2];
    switch (op) {
        case CRINIT_RTIMCMD_C_ADDTASK:
        case CRINIT_RTIMCMD_C_ADDSERIES:
        case CRINIT_RTIMCMD_C_ENABLE:
        case CRINIT_RTIMCMD_C_DISABLE:
        case CRINIT_RTIMCMD_C_STOP:
        case CRINIT_RTIMCMD_C_KILL:
        case CRINIT_RTIMCMD_C_RESTART:
        case CRINIT_RTIMCMD_C_NOTIFY:
            /*
             * Only allow the user running the crinit daemon to use these commands. With both
             * processes having the same effective user ID, the calling process already has the
             * privileges to execute any action specified by a crinit task.
             */
            return passedCreds->uid == geteuid();
        case CRINIT_RTIMCMD_C_STATUS:
        case CRINIT_RTIMCMD_C_TASKLIST:
        case CRINIT_RTIMCMD_C_GETVER:
            return true;
        case CRINIT_RTIMCMD_C_SHUTDOWN:
            if (crinitProcCapget(capdata, passedCreds->pid) == -1) {
                crinitErrPrint("Could not get process capabilities.");
                return false;
            }
            return (capdata[CAP_TO_INDEX(CAP_SYS_BOOT)].effective & CAP_TO_MASK(CAP_SYS_BOOT)) != 0;
        case CRINIT_RTIMCMD_R_ADDTASK:
        case CRINIT_RTIMCMD_R_ADDSERIES:
        case CRINIT_RTIMCMD_R_ENABLE:
        case CRINIT_RTIMCMD_R_DISABLE:
        case CRINIT_RTIMCMD_R_STOP:
        case CRINIT_RTIMCMD_R_KILL:
        case CRINIT_RTIMCMD_R_RESTART:
        case CRINIT_RTIMCMD_R_NOTIFY:
        case CRINIT_RTIMCMD_R_STATUS:
        case CRINIT_RTIMCMD_R_TASKLIST:
        case CRINIT_RTIMCMD_R_GETVER:
        case CRINIT_RTIMCMD_R_SHUTDOWN:
        default:
            crinitErrPrint("Unknown or unsupported opcode.");
            return false;
    }

    return false;
}

static inline int crinitProcCapget(cap_user_data_t out, pid_t pid) {
    struct __user_cap_header_struct caphdr = {_LINUX_CAPABILITY_VERSION_3, pid};
    if (syscall(SYS_capget, &caphdr, out) == -1) {
        crinitErrPrint("Could not get capabilities of process with PID %d.", pid);
        return -1;
    }
    return 0;
}
