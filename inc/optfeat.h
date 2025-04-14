// SPDX-License-Identifier: MIT
/**
 * @file optfeat.h
 * @brief Header related to optional behavior dependent on available system features.
 */
#ifndef __OPTFEAT_H__
#define __OPTFEAT_H__

/**
 * Hook types.
 */
typedef enum crinitHookType {
    CRINIT_HOOK_INIT,        ///< Initialization of the optional feature (eg. setup database).
    CRINIT_HOOK_EXIT,        ///< Cleanup of the optional feature (remove temporary files).
    CRINIT_HOOK_START,       ///< The optional feature is triggered by a specific event.
    CRINIT_HOOK_STOP,        ///< The optional feature is removed due to another event happening.
    CRINIT_HOOK_TASK_ADDED,  ///< Hook handles the addition of a new task.
} crinitHookType_t;

/**
 * Hook to be called whenever a new feature is provided by a task.
 *
 * Meant to be used to let Crinit change its own behavior whenever a relevant (optional or delayed) feature needed for
 * some special functionality gets provided.
 *
 * Currently only handles activation of syslog functionality if series config option USE_SYSLOG is true and a task
 * provides `syslog`.
 *
 * @param sysFeatName  Name of the newly-provided feature.
 * @param type         Type of the invoked hook.
 * @param data         Hook payload.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitFeatureHook(const char *sysFeatName, crinitHookType_t type, void *data);

#endif /*__OPTFEAT_H__ */
