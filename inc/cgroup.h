// SPDX-License-Identifier: MIT
/**
 * @file cgroup.h
 * @brief Header defining the data structures for cgroup support
 */
#ifndef __CGROUP_H__
#define __CGROUP_H__

#ifdef ENABLE_CGROUP

#include <fcntl.h>
#include <stddef.h>

#ifndef CRINIT_CGROUP_PATH
#define CRINIT_CGROUP_PATH "/sys/fs/cgroup"
#endif

/** Type to store a single cgroup configuration value **/
typedef struct {
    char *filename;  ///< target filename
    char *option;    ///< configuration to write to the target file
} crinitCgroupParam_t;

/** Type to store a complete configuration for a single cgroup **/
typedef struct {
    crinitCgroupParam_t *param;   ///< array with configuration parameters
    size_t paramCount;            ///< number of elements in param
} crinitCgroupConfiguration_t;

/** Type to store a single cgroup **/
struct crinitCgroup;
typedef struct crinitCgroup crinitCgroup_t;

struct crinitCgroup {
    char *name;                           ///< cgroup name
    int groupFd;                          ///< fd for /sys/fs/cgroup/<group>
    crinitCgroup_t *parent;               ///< parent cgroup in /sys/fs/cgroup/
    crinitCgroupConfiguration_t *config;  ///< pointer to cgroup configuration
};

/**
 * @brief Releases the memory held by param
 * @param param Pointer to crinitCGroupParam struct
 * @return On sucess 0, otherwise -1
 */
int crinitFreeCgroupParam(crinitCgroupParam_t *param);

/**
 * @brief Releases the memory held by config
 * @param config Pointer to crinitCgroupConfiguration struct
 * @return On sucess 0, otherwise -1
 */
int crinitFreeCgroupConfiguration(crinitCgroupConfiguration_t *config);

/**
 * @brief Releases the memory held by cgroup
 * @param cgroup Pointer to crinitCgroup_t struct
 * @return  On success 0, otherwise -1
 */
int crinitFreeCgroup(crinitCgroup_t *cgroup);

/**
 * @brief Copy one cgroup param structure to another
 * @param orig Pointer to original struct
 * @param out Pointer to target struct
 * @return On success 0, otherwise -1
 */
int crinitCopyCgroupParam(crinitCgroupParam_t *orig, crinitCgroupParam_t *out);

/**
 * @brief Convert string of the form "key=value" to a cgroup parameter struct.
 * @param in Pointer to source string
 * @param out Pointer to target struct
 * @return On success 0, otherwise -1
 */
int crinitCgroupConvertSingleParamToObject(const char *in, crinitCgroupParam_t *out);

/**
 * @brief Copy a cgroup configuration object
 * @param orig Pointer to orignal struct
 * @param out Pointer to target struct
 * @return On success 0, otherwise -1
 */
int crinitCopyCgroupConfiguration(crinitCgroupConfiguration_t *orig, crinitCgroupConfiguration_t *out);

/**
 * @brief Copy a cgroup definition
 * @param orig Pointer to input
 * @param out Pointer to target
 * @return On success 0, otherwise -1
 */
int crinitCopyCgroup(crinitCgroup_t *orig, crinitCgroup_t *out);

/**
 * @brief Converts a configuration array as returned by crinitConfConvToStrArr() to crinitCgroupConfiguration structure.
 *
 * The function will allocate memory for the sub structures in crinitCgroupConfiguration. It is the oblication of
 * the calling function to free those elements after use.
 *
 * @param confArray Input array
 * @param confArraySize Number of elements in confArray
 * @param result Pointer to result structure
 * @return On sucess 0, otherwise -1
 */
int crinitConvertConfigArrayToCGroupConfiguration(char **confArray, const int confArraySize,
                                                  crinitCgroupConfiguration_t *result);

/** @brief Finds the cgroup object by the cgroup's name
 * @param cgroups Pointer to cgroup storage
 * @param cgroupsCount Number of elements in cgroups
 * @param name Search string
 * @return If found, a pointer to the configuration element is returned, otherwise NULL.
 */
crinitCgroup_t *crinitFindCgroupByName(crinitCgroup_t **cgroups, size_t cgroupsCount, const char *name);

/**
 * @brief Configure a cgroup directory by applying a list of settings.
 *
 * Opens (and, if necessary, creates) the cgroup named by @p cgroup->name,
 * then iterates over @p cgroup->config.param (with @p cgroup->config.paramCount
 * elements) and writes each <filename,option> pair into the corresponding file
 * within that cgroup. All opened descriptors
 * are closed before return.
 *
 * @param[in] cgroup        Pointer to a crinitCgroupParam_t that holds a valid, non-empty name
 *                          and config.
 *                          Must not be NULL.
 *
 * @return On sucess 0, otherwise -1
 */
int crinitCGroupConfigure(crinitCgroup_t *cgroup);

/**
 * @brief Assign a process to a cgroup by writing its PID to @c cgroup.procs.
 *
 * Opens (but does not create) the cgroup directory named by @p cgroup->name
 * and writes the ASCII PID @p pid to its @c cgroup.procs file. This moves the
 * entire process into the target cgroup. All opened descriptors
 * are closed before return.
 *
 * Intended usage:  call only from crinit launcher after the process
 *                  and the cgroup exists.
 *
 * @param[in] cgroup    Pointer to a crinitCgroupParam_t that holds a valid, non-empty name
 *  *                   Must not be NULL.
 *
 * @param[in] pid       ID of a running process.
 *
 * @return On sucess 0, otherwise -1
 */
int crinitCGroupAssignPID(crinitCgroup_t *cgroup, pid_t pid);

/**
 * @brief Create all global cgroups (includes root cgroup if configured)
 * @return On sucess 0, otherwise -1
 */
int crinitCreateGlobalCGroups(void);

/**
 * @brief Splits a configuration line for global cgroup parameters into cgroup name and the parameter string itself
 * @param val Pointer to input line
 * @param name Output pointer with the cgroup's name. Caller needs to free the memory.
 * @param param Output pointer with the parameter string. Caller needs to free the memory.
 * @return On sucess 0, otherwise -1
 */
int crinitCgroupGlobalParamSplitNameAndParam(const char *val, char **name, char **param);

/**
 * @brief Tests if the given cgroup name is the name of a global cgroup
 * @param name Name to check
 * @param isGlobal Pointer to answer. If not global answer is 0. All other values indicate a global cgroup.
 * @return On sucess 0, otherwise -1
 */
int crinitCgroupNameIsGlobalCgroup(const char *name, int *isGlobal);

#endif

#endif
