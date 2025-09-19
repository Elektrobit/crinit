// SPDX-License-Identifier: MIT
/**
 * @file elosdep.h
 * @brief Header related to elos connection.
 */
#ifndef __ELOSDEP_H__
#define __ELOSDEP_H__

#include <stdbool.h>

struct crinitTask;
struct crinitTaskDB;

#define CRINIT_ELOSDEP_FEATURE_NAME "elos"

/**
 * Hook which will be invoked if a new task has been added and
 * will register the elos filters for this task.
 *
 * Modifies errno.
 *
 * @param task Task that has been added to elos.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosdepTaskAdded(struct crinitTask *task);

/**
 * Specify if Elos should be used.
 *
 * By default, Crinit will not connect to elos. If this is set to `true`, however, Crinit will connect to the
 * elos daemon.
 *
 * @param taskDb Task database to be informed if event occurs.
 * @param e      `true` if elos should be used, `false` otherwise.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int crinitElosdepActivate(struct crinitTaskDB *taskDb, bool e);

#endif /* __ELOSDEP_H__ */
