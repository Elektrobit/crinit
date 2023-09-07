// SPDX-License-Identifier: MIT
/**
 * @file elos-common.h
 * @brief Header related to elos connection.
 */
#ifndef __ELOS_COMMON_H__
#define __ELOS_COMMON_H__

#include <pthread.h>
#include <safu/common.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "logio.h"
#include "thrpool.h"

#define CRINIT_ELOS_LIBRARY "libelos.so.0"  ///< Elos shared library name

#define ELOS_ID_INVALID 0  ///< Invalid elos event queue ID constant.

#define ELOS_CLASSIFICATION_ELOS_MASK 0x00000000FFFFFFFFULL
#define ELOS_CLASSIFICATION_USER_MASK 0x000000FF00000000ULL
#define ELOS_CLASSIFICATION_RESERVED_MASK 0xFFFFFF0000000000ULL
#define ELOS_CLASSIFICATION_UNDEFINED 0x0000000000000000ULL
#define ELOS_CLASSIFICATION_KERNEL 0x0000000000000001ULL
#define ELOS_CLASSIFICATION_NETWORK 0x0000000000000002ULL
#define ELOS_CLASSIFICATION_SECURITY 0x0000000000000004ULL
#define ELOS_CLASSIFICATION_POWER 0x0000000000000008ULL
#define ELOS_CLASSIFICATION_STORAGE 0x0000000000000010ULL
#define ELOS_CLASSIFICATION_PROCESS 0x0000000000000020ULL
#define ELOS_CLASSIFICATION_IPC 0x0000000000000040ULL
#define ELOS_CLASSIFICATION_HARDWARE 0x0000000000000080ULL
#define ELOS_CLASSIFICATION_ELOS 0x0000000000000100ULL
#define ELOS_CLASSIFICATION_PROCESS_ERRORS 0x0000000000000200ULL
#define ELOS_CLASSIFICATION_USER_0 0x0000000100000000ULL
#define ELOS_CLASSIFICATION_USER_1 0x0000000200000000ULL
#define ELOS_CLASSIFICATION_USER_2 0x0000000400000000ULL
#define ELOS_CLASSIFICATION_USER_3 0x0000000800000000ULL
#define ELOS_CLASSIFICATION_USER_4 0x0000001000000000ULL
#define ELOS_CLASSIFICATION_USER_5 0x0000002000000000ULL
#define ELOS_CLASSIFICATION_USER_6 0x0000004000000000ULL
#define ELOS_CLASSIFICATION_USER_7 0x0000008000000000ULL

/**
 * Type defition for elos event queue IDs.
 */
typedef uint32_t crinitElosEventQueueId_t;

/**
 * Elos session type.
 */
typedef struct crinitElosSession {
    int fd;          ///< Connection socket file descriptor
    bool connected;  ///< Connection state
} crinitElosSession_t;

/**
 * Information of the sender sending an event to elos.
 */
typedef struct crinitElosEventSource {
    char *appName;   ///< Name of the application sending the event
    char *fileName;  ///< Filename sending the event
    pid_t pid;       ///< Id of the event sending process (0 for crinit)
} crinitElosEventSource_t;

/**
 * Possible severities an elos event can have.
 */
typedef enum crinitElosSeverityE_t {
    ELOS_SEVERITY_OFF = 0,
    ELOS_SEVERITY_FATAL,
    ELOS_SEVERITY_ERROR,
    ELOS_SEVERITY_WARN,
    ELOS_SEVERITY_INFO,
    ELOS_SEVERITY_DEBUG,
    ELOS_SEVERITY_VERBOSE,
} crinitElosSeverityE_t;

typedef int crinitElosEventMessageCodeE_t;

/**
 * Event send to elos.
 */
typedef struct crinitElosEvent {
    struct timespec date;            ///< Unix timestamp in nano seconds resolution.
    crinitElosEventSource_t source;  ///< Struct containing informations about where the event originated from
    crinitElosSeverityE_t
        severity;      ///< When the message gets classified it is mapped to a severity value defined by this project.
    char *hardwareid;  ///< An unique identifier for the hardware which produced this particular information.
    uint64_t classification;  ///< Used to categorize events to be related to a certain context of system information.
    crinitElosEventMessageCodeE_t messageCode;  ///< Used to hide information, thus an information has a meaning without
                                                ///< providing a payload (text).
    char *payload;                              ///< Actual payload of the information.
} crinitElosEvent_t;

/**
 * Elos event vector type.
 */
typedef struct crinitElosEventVector {
    size_t memorySize;      ///< Size of memory used
    size_t elementSize;     ///< Size of a single element
    uint32_t elementCount;  ///< Number of elements in the event vector
    void *data;             ///< Continous data block holding all elements
} crinitElosEventVector_t;

/**
 * Elos virtual table and connection data.
 */
typedef struct crinitElosVirtualTable {
    char *elosServer;  ///< Elos server name or ip
    int elosPort;      ///< Elos server port

    safuResultE_t (*connect)(const char *, uint16_t,
                             crinitElosSession_t **);  ///< Function pointer to the elosConnectTcpip function
    safuResultE_t (*getVersion)(crinitElosSession_t *,
                                const char **);  ///< Function pointer to the elosGetVersion function
    safuResultE_t (*eventSubscribe)(
        crinitElosSession_t *, const char *[], size_t,
        crinitElosEventQueueId_t *);  ///< Function pointer to the elosEventSubscribe function
    safuResultE_t (*eventUnsubscribe)(
        crinitElosSession_t *, crinitElosEventQueueId_t);  ///< Function pointer to the elosEventUnsubscribe function
    safuResultE_t (*eventQueueRead)(
        crinitElosSession_t *, crinitElosEventQueueId_t,
        crinitElosEventVector_t **);                            ///< Function pointer to the elosEventQueueRead function
    void *(*eventVecGetLast)(const crinitElosEventVector_t *);  ///< Function pointer to the safuVecGetLast function
    void (*eventVectorDelete)(crinitElosEventVector_t *);  ///< Function pointer to the elosEventVectorDelete function
    safuResultE_t (*eventPublish)(crinitElosSession_t *,
                                  const crinitElosEvent_t *);  ///< Function pointer to the elosEventPublish function
    safuResultE_t (*disconnect)(crinitElosSession_t *);  ///< Function pointer to the elosDisconnect function
} crinitElosVirtualTable_t;

/**
 * Initializes the elos vtable.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosInit(void);

/**
 * Returns the internal elos virtual table.
 */
crinitElosVirtualTable_t *crinitElosGetVTable(void);

/**
 * Disconnect from elos daemon.
 *
 * @param session Session to disconnect.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosDisconnect(crinitElosSession_t *session, pthread_mutex_t *sessionLock);

/**
 * Macro to simplify checking for a valid elos session.
 *
 * Will print an error message and return from the calling function with an error code if the
 * session pointer is either null or the conn.
 *
 * HINT: This uses a GNU extension of gcc to define a compound statement enclosed in parentheses.
 *
 * @param session       Elos session to be used.
 * @param sessionLock   The session lock.
 * @param func          Elos function to be called.
 * @param err_msg       Error message to be returned on error.
 */
#define crinitElosTryExec(session, sessionLock, func, err_msg, ...)                                               \
    __extension__({                                                                                               \
        int res = SAFU_RESULT_OK;                                                                                 \
                                                                                                                  \
        if ((errno = pthread_mutex_lock(sessionLock)) != 0) {                                                     \
            crinitErrnoPrint("Failed to lock elos session.");                                                     \
            res = -1;                                                                                             \
        } else {                                                                                                  \
            while ((session) == NULL || !(session)->connected) {                                                  \
                if (crinitElosGetVTable()->elosServer == NULL) {                                                  \
                    crinitErrPrint("Elos server configuration missing or not loaded yet.");                       \
                    res = -1;                                                                                     \
                    break;                                                                                        \
                } else {                                                                                          \
                    if ((session) != NULL) {                                                                      \
                        crinitElosGetVTable()->disconnect(session);                                               \
                    }                                                                                             \
                                                                                                                  \
                    res = crinitElosGetVTable()->connect(crinitElosGetVTable()->elosServer,                       \
                                                         crinitElosGetVTable()->elosPort,                         \
                                                         (crinitElosSession_t **)&(session));                     \
                    if (res != SAFU_RESULT_OK) {                                                                  \
                        crinitErrPrint("Failed to connect to elosd on %s:%d.", crinitElosGetVTable()->elosServer, \
                                       crinitElosGetVTable()->elosPort);                                          \
                        usleep(500000);                                                                           \
                    }                                                                                             \
                }                                                                                                 \
            }                                                                                                     \
                                                                                                                  \
            if (res == SAFU_RESULT_OK) {                                                                          \
                res = func(__VA_ARGS__);                                                                          \
                if (res != SAFU_RESULT_OK) {                                                                      \
                    crinitErrPrint(err_msg);                                                                      \
                }                                                                                                 \
                                                                                                                  \
                if ((errno = pthread_mutex_unlock(sessionLock)) != 0) {                                           \
                    crinitErrnoPrint("Failed to unlock elos session.");                                           \
                    res = -1;                                                                                     \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
                                                                                                                  \
        res;                                                                                                      \
    })

#endif /* __ELOS_COMMON_H__ */
