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
#include <linux/capability.h>
#include <signal.h>
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
#define EBCL_gettid() ((pid_t)syscall(SYS_gettid))

/** Maximum number of unserviced connections until the server starts refusing **/
#define MAX_CONN_BACKLOG 100

/** Helper structure defining the arguments to connThread() **/
typedef struct ebcl_ConnThrArgs_t {
    int sockFd;                ///< The socket to accept connections from.
    ebcl_ThreadPool_t *tpRef;  ///< Pointer to the ebcl_ThreadPool_t context, needed for
                               ///< EBCL_threadPoolThreadAvailCallback() and EBCL_threadPoolThreadBusyCallback().
} ebcl_ConnThrArgs_t;

static ebcl_ThreadPool_t EBCL_workers;  ///< The worker thread pool to run connThread() in.
static ebcl_TaskDB_t *EBCL_tdbRef;      ///< Pointer to the ebcl_TaskDB_t to operate on.

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
 * @param args  Arguments to the function, see ebcl_ConnThrArgs_t.
 *
 * @return  Does not return unless its thread is canceled in which case the return value is undefined.
 */
static void *EBCL_connThread(void *args);
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
static int EBCL_createSockFile(int *sockFd, const char *path);
/**
 * Checks if a received struct cmsghdr has expected length and contents.
 *
 * Expected is a struct cmsghdr containing SCM_CREDENTIALS.
 *
 * @param cmh  The header to check.
 *
 * @return   true if everything is as expected, false otherwise
 */
static inline bool EBCL_cmsgHdrCheck(const struct cmsghdr *cmh);
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
static inline bool EBCL_ucredCheckEqual(const struct ucred *a, const struct ucred *b);
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
static inline int EBCL_sendStr(int sockFd, const char *str);
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
static inline int EBCL_recvStr(int sockFd, char **str, struct ucred *passedCreds);
/**
 * Checks if given process credentials imply permission to execute remote command.
 *
 * @param op           The opcode of the command the process requests to execute.
 * @param passedCreds  The credentials of the requesting process obtained via SCM_CREDENTIALS.
 *
 * @return true if \a passedCreds imply the sending process is permitted to execute \a op. Returns false otherwise, also
 * on errors.
 */
static inline bool EBCL_checkPerm(ebcl_RtimOp_t op, const struct ucred *passedCreds);
/**
 * Gets capabilities of process specified by PID.
 *
 * @param out  Return pointer for capabilities. Note, that the Linux API defines cap_user_data_t as a pointer to
 *             struct __user_cap_header_struct. The given pointer needs to point to at least two elements.
 * @param pid  PID of the process from which to get the capabilities.
 *
 * @return 0 on success, -1 on error
 */
static inline int EBCL_procCapget(cap_user_data_t out, pid_t pid);

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
static inline int EBCL_mkdirp(char *pathname, mode_t mode);

int EBCL_startInterfaceServer(ebcl_TaskDB_t *ctx, const char *sockFile) {
    if (ctx == NULL || sockFile == NULL) {
        EBCL_errPrint("Given arguments must not be NULL.");
        return -1;
    }

    EBCL_tdbRef = ctx;
    char *sockFileTmp = strdup(sockFile);
    if (sockFileTmp == NULL) {
        EBCL_errnoPrint("Could not duplicate string.");
        return -1;
    }

    char *sockDir = dirname(sockFileTmp);
    if (EBCL_mkdirp(sockDir, 0777) == -1) {
        EBCL_errnoPrint("Could not create directory \'%s\'.", sockDir);
        free(sockFileTmp);
        return -1;
    }
    free(sockFileTmp);
    int sockFd = -1;
    umask(0);
    if (EBCL_createSockFile(&sockFd, sockFile) == -1) {
        EBCL_errPrint("Could not create socket file at \'%s\'.", sockFile);
        return -1;
    }
    umask(0022);
    ebcl_ConnThrArgs_t a = {sockFd, &EBCL_workers};
    if (EBCL_threadPoolInit(&EBCL_workers, 0, EBCL_connThread, &a, sizeof(ebcl_ConnThrArgs_t)) == -1) {
        EBCL_errPrint("Could not fill server thread pool.");
        return -1;
    }

    return 0;
}

static inline int EBCL_mkdirp(char *pathName, mode_t mode) {
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

static inline bool EBCL_cmsgHdrCheck(const struct cmsghdr *cmh) {
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

static void *EBCL_connThread(void *args) {
    pid_t threadId = EBCL_gettid();
    if (args == NULL) {
        EBCL_errPrint("(TID %d) Argument to connection worker thread must not be NULL.", threadId);
        return NULL;
    }
    ebcl_ConnThrArgs_t *a = (ebcl_ConnThrArgs_t *)args;
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
        if (EBCL_sendStr(connSockFd, "RTR") == -1) {
            EBCL_errPrint("(TID %d) Could not send RTR-message to client.", threadId);
            close(connSockFd);
            continue;
        }
        struct ucred msgCreds = {0};
        char *clientMsg = NULL;
        if (EBCL_recvStr(connSockFd, &clientMsg, &msgCreds) == -1) {
            EBCL_errPrint("(TID %d) Could not receive string message from client.", threadId);
            close(connSockFd);
            continue;
        }

        EBCL_dbgInfoPrint("(TID %d) Received string \'%s\' from client.", threadId, clientMsg);
        EBCL_dbgInfoPrint("(TID %d) Received following credentials from peer process: PID=%d, UID=%d, GID=%d", threadId,
                          msgCreds.pid, msgCreds.uid, msgCreds.gid);

        ebcl_RtimCmd_t cmd, res;
        if (EBCL_parseRtimCmd(&cmd, clientMsg) == -1) {
            EBCL_errPrint("(TID %d) Could not parse command from client.", threadId);
            free(clientMsg);
            close(connSockFd);
            continue;
        }
        free(clientMsg);
        if (!EBCL_checkPerm(cmd.op, &msgCreds)) {
            EBCL_errPrint("(TID %d) Client does not have permission to issue command.", threadId);
            if (EBCL_buildRtimCmd(&res, cmd.op + 1, 2, EBCL_RTIMCMD_RES_ERR, "Permission denied.") == -1) {
                EBCL_errPrint("Could not generate response to client.");
                EBCL_destroyRtimCmd(&cmd);
                close(connSockFd);
                continue;
            }
        } else {
            if (EBCL_execRtimCmd(EBCL_tdbRef, &res, &cmd) == -1) {
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
        if (EBCL_sendStr(connSockFd, resStr) == -1) {
            EBCL_errPrint("(TID %d) Could not send response message to client.", threadId);
            close(connSockFd);
            continue;
        }

        close(connSockFd);
        EBCL_threadPoolThreadAvailCallback(a->tpRef);
    }
    return NULL;
}

static int EBCL_createSockFile(int *sockFd, const char *path) {
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

static inline int EBCL_sendStr(int sockFd, const char *str) {
    pid_t threadId = EBCL_gettid();
    if (str == NULL) {
        EBCL_errPrint("(TID %d) String to send must not be NULL.", threadId);
        return -1;
    }

    size_t dataLen = strlen(str) + 1;
    if (send(sockFd, &dataLen, sizeof(size_t), MSG_NOSIGNAL) == -1) {
        EBCL_errnoPrint("(TID %d) Could not send length packet (\'%zu\') of string \'%s\' to client. %d", threadId,
                        dataLen, str, sockFd);
        return -1;
    }

    if (send(sockFd, str, dataLen, MSG_NOSIGNAL) == -1) {
        EBCL_errnoPrint("(TID %d) Could not send string \'%s\' to client.", threadId, str);
        return -1;
    }

    return 0;
}

static inline int EBCL_recvStr(int sockFd, char **str, struct ucred *passedCreds) {
    pid_t threadId = EBCL_gettid();
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
    EBCL_dbgInfoPrint("(TID %d) Received message of %d Bytes. Content:\n\'%zu\'", threadId, bytesRead, dataLen);

    struct cmsghdr *cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!EBCL_cmsgHdrCheck(cmHdr)) {
        EBCL_errPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        return -1;
    }
    memcpy(&scmCreds[0], CMSG_DATA(cmHdr), sizeof(struct ucred));

    *str = malloc(dataLen);
    if (*str == NULL) {
        EBCL_errnoPrint("(TID %d) Could not allocate receive buffer of size %zu Bytes.", threadId, dataLen);
        return -1;
    }
    iov.iov_base = *str;
    iov.iov_len = dataLen;

    bytesRead = recvmsg(sockFd, &mHdr, 0);
    if (bytesRead < 0) {
        EBCL_errnoPrint("(TID %d) Could not receive string data message of size %zu Bytes via socket.", threadId,
                        dataLen);
        goto fail;
    }
    if ((size_t)bytesRead != dataLen) {
        EBCL_errPrint("Received data of unexpected length from client: %ld Bytes vs. announced %zu Bytes ", bytesRead,
                      dataLen);
        goto fail;
    }
    // force terminating zero
    (*str)[dataLen - 1] = '\0';
    EBCL_dbgInfoPrint("(TID %d) Received message of %d Bytes. Content:\n\'%s\'", threadId, bytesRead, *str);

    cmHdr = CMSG_FIRSTHDR(&mHdr);
    if (!EBCL_cmsgHdrCheck(cmHdr)) {
        EBCL_errPrint("(TID %d) Control message header of received ancillary data is invalid.", threadId);
        goto fail;
    }
    memcpy(&scmCreds[1], CMSG_DATA(cmHdr), sizeof(struct ucred));

    if (!EBCL_ucredCheckEqual(&scmCreds[0], &scmCreds[1])) {
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

static inline bool EBCL_ucredCheckEqual(const struct ucred *a, const struct ucred *b) {
    return (a->pid == b->pid) && (a->uid == b->uid) && (a->gid == b->gid);
}

static inline bool EBCL_checkPerm(ebcl_RtimOp_t op, const struct ucred *passedCreds) {
    if (passedCreds == NULL) {
        EBCL_errPrint("Pointer arguments must not be NULL.");
        return false;
    }

    struct __user_cap_data_struct capdata[2];
    switch (op) {
        case EBCL_RTIMCMD_C_ADDTASK:
        case EBCL_RTIMCMD_C_ADDSERIES:
        case EBCL_RTIMCMD_C_ENABLE:
        case EBCL_RTIMCMD_C_DISABLE:
        case EBCL_RTIMCMD_C_STOP:
        case EBCL_RTIMCMD_C_KILL:
        case EBCL_RTIMCMD_C_RESTART:
        case EBCL_RTIMCMD_C_NOTIFY:
            return passedCreds->uid == 0;
        case EBCL_RTIMCMD_C_STATUS:
            return true;
        case EBCL_RTIMCMD_C_SHUTDOWN:
            if (EBCL_procCapget(capdata, passedCreds->pid) == -1) {
                EBCL_errPrint("Could not get process capabilities.");
                return false;
            }
            return (capdata[CAP_TO_INDEX(CAP_SYS_BOOT)].effective & CAP_TO_MASK(CAP_SYS_BOOT)) != 0;
        case EBCL_RTIMCMD_R_ADDTASK:
        case EBCL_RTIMCMD_R_ADDSERIES:
        case EBCL_RTIMCMD_R_ENABLE:
        case EBCL_RTIMCMD_R_DISABLE:
        case EBCL_RTIMCMD_R_STOP:
        case EBCL_RTIMCMD_R_KILL:
        case EBCL_RTIMCMD_R_RESTART:
        case EBCL_RTIMCMD_R_NOTIFY:
        case EBCL_RTIMCMD_R_STATUS:
        case EBCL_RTIMCMD_R_SHUTDOWN:
        default:
            EBCL_errPrint("Unknown or unsupported opcode.");
            return false;
    }

    return false;
}

static inline int EBCL_procCapget(cap_user_data_t out, pid_t pid) {
    struct __user_cap_header_struct caphdr = {_LINUX_CAPABILITY_VERSION_3, pid};
    if (syscall(SYS_capget, &caphdr, out) == -1) {
        EBCL_errPrint("Could not get capabilities of process with PID %d.", pid);
        return -1;
    }
    return 0;
}

