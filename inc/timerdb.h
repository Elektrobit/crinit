// SPDX-License-Identifier: MIT
#ifndef __TIMER_DB_H__
#define __TIMER_DB_H__

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "taskdb.h"
#include "timer.h"

/**
 * the initial capacity for the crinit timer db.
 */
#define TIMER_DB_INITIAL_CAP 256

/**
 * the type for the crinit timer db.
 */
typedef struct crinitTimerDB {
    size_t cap;
    size_t size;
    crinitTimer_t *timerList;
    struct pollfd *pollList;
    crinitTaskDB_t *taskDB;
    pthread_t timerThread;
    pthread_mutex_t lock;  ///< Mutex to lock the TimerDB, shall be used for any operations on the data structure if
} crinitTimerDB_t;

/**
 * Initialize the timer db handling all of crinit's timers.
 *
 * @param taskDB  the task db to initialize the timerdb for
 *
 * @return 0 on success, -1 on error
 */
int crinitTimerDBInit(crinitTaskDB_t *taskDB);
/**
 * Spawns the timer thread handling all of crinits timer.
 *
 * @return 0 on success, -1 on error
 */
int crinitTimerDBSpawn(void);

/**
 * Adds a timer to crinits timerDB.
 *
 * @param timerStr  the configuration string/name for the timer
 */
void crinitTimerDBAddTimer(char *timerStr);
/**
 * Removes a timer from crinits timerDB.
 *
 * @param timerStr  the configuration string/name for the timer
 */
void crinitTimerDBRemoveTimer(char *timerStr);

#endif /* __TIMER_DB_H__ */
