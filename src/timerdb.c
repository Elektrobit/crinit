// SPDX-License-Identifier: MIT
/**
 * @file timerdb.c
 * @brief Implementation of functions related managing the pool of all timers.
 */
#include "timerdb.h"

#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "common.h"
#include "logio.h"
#include "taskdb.h"
#include "timer.h"

crinitTimerDB_t crinitTimerPool = {0};

/**
 * The TimerDB thread function that handles the triggering of all timer events.
 *
 * @param args   UNUSED
 */
static void *crinitTimerDBRunPool(void *args);
/**
 * Debug print the state of the timer poll and all timers in it.
 *
 * @param pool  reference to the timerDB
 */
static void crinitPrintTimerPool(crinitTimerDB_t *pool);
/**
 * Insert a timer into the timerDB.
 * The timer needs to be fully initialized including the next timestamp
 *
 * @param timer  the timer to insert
 *
 * @return 0 on success, -1 on error
 */
static int crinitTimerDBInsertTimer(crinitTimer_t timer);

int crinitTimerDBInit(crinitTaskDB_t *taskDB) {
    crinitNullCheck(-1, taskDB);
    crinitInfoPrint("Initializing TimerDB");
    crinitTimerPool.cap = TIMER_DB_INITIAL_CAP;
    crinitTimerPool.timerList = calloc(TIMER_DB_INITIAL_CAP, sizeof(crinitTimer_t));
    if (crinitTimerPool.timerList == NULL) {
        crinitErrnoPrint("Could not allocate memory for TimerDB.");
        crinitTimerPool.cap = 0;
        return -1;
    }
    crinitTimerPool.pollList = calloc(TIMER_DB_INITIAL_CAP, sizeof(struct pollfd));
    if (crinitTimerPool.pollList == NULL) {
        crinitErrnoPrint("Could not allocate memory for TimerDB.");
        goto failPollList;
    }
    crinitTimerPool.size = 0;

    crinitTimerPool.pollList[0].fd = eventfd(0, 0);
    if (crinitTimerPool.pollList[0].fd == -1) {
        crinitErrnoPrint("Could not get eventfd for TimerDB.");
        goto fail;
    }
    crinitTimerPool.taskDB = taskDB;
    crinitTimerPool.pollList[0].events = POLLIN;
    crinitTimerPool.size = 1;

    if ((errno = pthread_mutex_init(&crinitTimerPool.lock, NULL)) != 0) {
        crinitErrnoPrint("Could not initialize mutex for TimerDB.");
        goto fail;
    }
    return 0;

fail:
    free(crinitTimerPool.pollList);
    crinitTimerPool.pollList = NULL;
failPollList:
    free(crinitTimerPool.timerList);
    crinitTimerPool.timerList = NULL;
    crinitTimerPool.cap = 0;
    crinitTimerPool.size = 0;
    return -1;
}

static void *crinitTimerDBRunPool(void *args) {
    CRINIT_PARAM_UNUSED(args);
    while (1) {
        int ready = poll(crinitTimerPool.pollList, crinitTimerPool.size, -1);
        if (ready == -1) {
            crinitErrnoPrint("polling failed.");
            return NULL;
        }
        if ((errno = pthread_mutex_lock(&crinitTimerPool.lock)) != 0) {
            crinitErrnoPrint("Could not queue up for mutex lock.");
            return NULL;
        }
        uint64_t u = 0;
        if (crinitTimerPool.pollList[0].revents & POLLIN) {
            if (read(crinitTimerPool.pollList[0].fd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
                crinitErrnoPrint("Couldn't read update events.");
            }
            pthread_mutex_unlock(&crinitTimerPool.lock);
            continue;
        } else if (crinitTimerPool.pollList[0].revents != 0) {
            crinitErrnoPrint("Couldn't poll update events.");
        }
        for (size_t i = 0; i < crinitTimerPool.size; i++) {
            if (crinitTimerPool.pollList[i].revents == POLLIN) {
                if (read(crinitTimerPool.pollList[i].fd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
                    crinitErrnoPrint("Couldn't read timer.");
                }
                crinitTaskDep_t dep = {.name = "@timer", .event = crinitTimerPool.timerList[i].name};
                crinitTaskDBFulfillDep(crinitTimerPool.taskDB, &dep, NULL);
                struct timespec ts = crinitTimerPool.timerList[i].next.it_value;

                crinitTimerPool.timerList[i].next.it_value =
                    crinitTimerNextTime(&ts, &crinitTimerPool.timerList[i].def);

                if (timerfd_settime(crinitTimerPool.pollList[i].fd, TFD_TIMER_ABSTIME,
                                    &crinitTimerPool.timerList[i].next, NULL) == -1) {
                    char buf[100];
                    crinitSPrintTimerDef(buf, &crinitTimerPool.timerList->def);
                    crinitErrnoPrint("Couldn't reset timer %s.", buf);
                }
            } else if (crinitTimerPool.pollList[i].revents != 0) {
                crinitErrnoPrint("Couldn't poll timer.");
            }
        }
        pthread_mutex_unlock(&crinitTimerPool.lock);
    }
    return NULL;
}

static void crinitPrintTimerPool(crinitTimerDB_t *pool) {
    crinitInfoPrint("TimerPool: size=%ld  cap=%ld", pool->size, pool->cap);
    for (size_t i = 1; i < pool->size; i++) {
        char buff[100];
        crinitSPrintTimerDef(buff, &pool->timerList[i].def);
        crinitInfoPrint("timer[%lu]:", i);
        crinitInfoPrint("    def=%s", buff);

        struct tm t = {0};
        crinitZonedTimeR(&pool->timerList[i].next.it_value.tv_sec, pool->timerList[i].def.timezone, &t);
        strftime(buff, 100, "%F %H:%M:%S %z", &t);
        crinitInfoPrint("    name='%s'  refs=%lu  next=%s", pool->timerList[i].name, pool->timerList[i].refs, buff);
    }
}

int crinitTimerDBSpawn(void) {
    int res = 0;
    if ((errno = pthread_mutex_lock(&crinitTimerPool.lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    crinitPrintTimerPool(&crinitTimerPool);
    if (errno = (pthread_mutex_unlock(&crinitTimerPool.lock)) != 0) {
        crinitErrnoPrint("failed to unlock the timer pool.");
        return -1;
    }

    pthread_t timerThread = {0};
    pthread_create(&timerThread, NULL, crinitTimerDBRunPool, NULL);

    return res;
}

static int crinitTimerDBInsertTimer(crinitTimer_t timer) {
    int res = 0;
    if ((errno = pthread_mutex_lock(&crinitTimerPool.lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return -1;
    }
    if (crinitTimerPool.size >= crinitTimerPool.cap) {
        // TODO:  properly handle reallocating when list is full.
        crinitErrPrint("TimerDB has no space left for additionall timer!!");
        res = -1;
        goto end;
    }

    int fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1) {
        res = -1;
        goto end;
    }
    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &timer.next, NULL) == -1) {
        res = -1;
        goto end;
    }

    crinitTimerPool.timerList[crinitTimerPool.size] = timer;
    crinitTimerPool.pollList[crinitTimerPool.size].fd = fd;
    crinitTimerPool.pollList[crinitTimerPool.size].events = POLLIN;
    crinitTimerPool.size += 1;

    uint64_t u = 1;
    if (write(crinitTimerPool.pollList[0].fd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
        crinitErrPrint("failed to notify timerpool");
        res = -1;
    }

end:
    pthread_mutex_unlock(&crinitTimerPool.lock);
    return res;
}

void crinitTimerDBRemoveTimer(char *timerStr) {
    if ((errno = pthread_mutex_lock(&crinitTimerPool.lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return;
    }
    uint64_t u = 1;
    for (size_t i = 1; i < crinitTimerPool.size; i++) {
        if (0 == strcmp(crinitTimerPool.timerList[i].name, timerStr)) {
            if (write(crinitTimerPool.pollList[0].fd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
                crinitErrPrint("failed to notify timerpool");
            }
            if (crinitTimerPool.timerList[i].refs > 0) {
                crinitTimerPool.timerList[i].refs -= 1;
            }
            if (crinitTimerPool.timerList[i].refs == 0) {
                if (i < crinitTimerPool.size - 1) {
                    free(crinitTimerPool.timerList[i].name);
                    crinitTimerPool.timerList[i] = crinitTimerPool.timerList[crinitTimerPool.size - 1];
                    crinitTimerPool.pollList[i] = crinitTimerPool.pollList[crinitTimerPool.size - 1];
                }
                crinitTimerPool.size -= 1;
            }
        }
    }
    pthread_mutex_unlock(&crinitTimerPool.lock);
}

void crinitTimerDBAddTimer(char *timerStr) {
    if ((errno = pthread_mutex_lock(&crinitTimerPool.lock)) != 0) {
        crinitErrnoPrint("Could not queue up for mutex lock.");
        return;
    }
    for (size_t i = 1; i < crinitTimerPool.size; i++) {
        if (0 == strcmp(crinitTimerPool.timerList[i].name, timerStr)) {
            uint64_t u = 1;
            if (write(crinitTimerPool.pollList[0].fd, &u, sizeof(uint64_t)) != sizeof(uint64_t)) {
                crinitErrPrint("failed to notify timerpool");
            }
            crinitTimerPool.timerList[i].refs += 1;
            if (pthread_mutex_unlock(&crinitTimerPool.lock) != 0) {
                crinitErrnoPrint("error unlocking mutex.");
            }
            return;
        }
    }
    if (pthread_mutex_unlock(&crinitTimerPool.lock) != 0) {
        crinitErrnoPrint("error unlocking mutex in add.");
    }

    crinitTimer_t timer = {0};
    timer.name = strdup(timerStr);
    crinitTimerParse(timer.name, &(timer.def));

    struct timespec ti;
    timespec_get(&ti, TIME_UTC);

    ti = crinitTimerNextTime(&ti, &(timer.def));
    timer.refs = 1;
    timer.next.it_value = ti;
    timer.next.it_interval.tv_sec = 0;
    timer.next.it_interval.tv_nsec = 0;

    if (crinitTimerDBInsertTimer(timer) == -1) {
        crinitErrPrint("Failed to insert Timer @timer:%s into TimerDB", timerStr);
    } else {
        crinitDbgInfoPrint("Successfully inserted Timer @timer:%s into TimerDB", timerStr);
    }
}
