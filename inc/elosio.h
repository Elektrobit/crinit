// SPDX-License-Identifier: MIT
/**
 * @file elosio.h
 * @brief Header related to elos connection.
 */ 
#ifndef __ELOSIO_H__
#define __ELOSIO_H__

#include <stdbool.h>

struct crinitTask_t;
struct crinitTaskDB_t;

#define CRINIT_ELOSIO_FEATURE_NAME "elos"

/**
 * Hook which will be invoked if a new task has been added and
 * will register the elos filters for this task.
 *
 * @param task Task that has been added to elos.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosioTaskAdded(struct crinitTask_t *task);

/**
 * Specify if Elos should be used.
 *
 * By default, Crinit will not connect to elos. If this is set to `true`, however, Crinit will connect to the
 * elos daemon.
 *
 * @param taskDb Task database to be informed if event occurs.
 * @param sl     `true` if elos should be used, `false` otherwise.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosioActivate(struct crinitTaskDB_t *taskDb, bool sl);

#endif /* __ELOSIO_H__ */
