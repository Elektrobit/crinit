/**
 * @file optfeat.h
 * @brief Header related to optional behavior dependent on available system features.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __OPTFEAT_H__
#define __OPTFEAT_H__

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
 *
 * @return 0 on success, -1 otherwise
 */
int EBCL_crinitFeatureHook(const char *sysFeatName);

#endif /*__OPTFEAT_H__ */
