// SPDX-License-Identifier: MIT
/**
 * @file cgroup.h
 * @brief Header defining the data structures for cgroup support
 */
#ifndef __CGROUP_H__
#define __CGROUP_H__

#ifdef ENABLE_CGROUP

#include <stddef.h>

/** Type to store a single cgroup configuration value **/
typedef struct {
    char *filename;  ///< target filename
    char *option;    ///< configuration to write to the target file
} crinitCgroupParam_t;

/** Type to store a complete configuration for a single cgroup **/
typedef struct {
    crinitCgroupParam_t **param;  ///< array with configuration parameters
    size_t paramCount;            ///< number of elements in param
} crinitCgroupConfiguration_t;

/** Type to store a single cgroup **/
typedef struct {
    char *name;                           ///< cgroup name
    crinitCgroupConfiguration_t *config;  ///< pointer to cgroup configuration
} crinitCgroup_t;

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
 * @brief Copy an object of type crinitCgroupParam_t
 * @param orig Pointer to object to copy
 * @param out Pointer to target object
 * @return On sucess 0, otherwise -1
 */
int crinitCopyCgroupParam(crinitCgroupParam_t *orig, crinitCgroupParam_t *out);

/**
 * @brief Copy an object of type crinitCgroupConfiguration_t
 * @param orig Pointer to object to copy
 * @param out Pointer to target object
 * @return On sucess 0, otherwise -1
 */
int crinitCopyCgroupConfiguration(crinitCgroupConfiguration_t *orig, crinitCgroupConfiguration_t *out);

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

#endif

#endif
