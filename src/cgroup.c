// SPDX-License-Identifier: MIT
/**
 * @file cgroup.c
 * @brief Support function for cgroup support
 */

#ifdef ENABLE_CGROUP

#include "cgroup.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

#include "common.h"
#include "globopt.h"
#include "logio.h"

int crinitFreeCgroupParam(crinitCgroupParam_t *param) {
    crinitNullCheck(-1, param);

    free(param->filename);
    free(param->option);
    param->filename = NULL;
    param->option = NULL;

    return 0;
}

int crinitFreeCgroupConfiguration(crinitCgroupConfiguration_t *config) {
    crinitNullCheck(-1, config);

    if (config->param) {
        for (size_t i = 0; i < config->paramCount; i++) {
            if (config->param[i]) {
                crinitFreeCgroupParam(config->param[i]);
                free(config->param[i]);
                config->param[i] = NULL;
            }
        }
    }
    free(config->param);
    config->param = NULL;
    config->paramCount = 0;

    return 0;
}

int crinitFreeCgroup(crinitCgroup_t *cgroup) {
    crinitNullCheck(-1, cgroup);

    free(cgroup->name);
    cgroup->name = NULL;

    if (cgroup->config) {
        crinitFreeCgroupConfiguration(cgroup->config);
        free(cgroup->config);
        cgroup->config = NULL;
    }
    return 0;
}

int crinitCopyCgroupParam(crinitCgroupParam_t *orig, crinitCgroupParam_t *out) {
    crinitNullCheck(-1, orig, out);

    out->filename = strdup(orig->filename);
    out->option = strdup(orig->option);

    if (out->filename == NULL || out->option == NULL) {
        crinitFreeCgroupParam(out);
        return -1;
    }

    return 0;
}

int crinitCopyCgroupConfiguration(crinitCgroupConfiguration_t *orig, crinitCgroupConfiguration_t *out) {
    crinitNullCheck(-1, orig, out);

    out->paramCount = orig->paramCount;
    if (out->paramCount > 0) {
        out->param = calloc(sizeof(orig->param), orig->paramCount);
        if (out->param == NULL) {
            crinitErrPrint("Failed to allocate memory for config params.");
            return -1;
        }
        for (size_t i = 0; i < out->paramCount; i++) {
            out->param[i] = calloc(sizeof(*(orig->param)), 1);
            if (out->param[i] == NULL) {
                crinitErrPrint("Failed to allocate memory for single param.");
                goto fail;
            }
            if (crinitCopyCgroupParam(orig->param[i], out->param[i]) != 0) {
                goto fail;
            }
        }
    }

    return 0;

fail:
    crinitFreeCgroupConfiguration(out);
    return -1;
}

int crinitCopyCgroup(crinitCgroup_t *orig, crinitCgroup_t *out) {
    crinitNullCheck(-1, orig, out);

    out->config = calloc(sizeof(*out->config), 1);
    if (out->config == NULL) {
        crinitErrPrint("Failed to allocate memory for cgroup configuration while copying cgroup.");
        return -1;
    }
    if (crinitCopyCgroupConfiguration(orig->config, out->config) == -1) {
        crinitErrPrint("Failed to copy configuration part while copying cgroup.");
        return -1;
    }
    out->groupFd = orig->groupFd;
    out->parent = orig->parent;
    out->name = strdup(orig->name);
    if (out->name == NULL) {
        crinitErrPrint("Failed to copy name while copying cgroup.");
        crinitFreeCgroupConfiguration(out->config);
        return -1;
    }

    return 0;
}

int crinitCgroupConvertSingleParamToObject(char *in, crinitCgroupParam_t *out) {
    crinitNullCheck(-1, in, out);

    char *delim = NULL;
    if ((delim = strchr(in, '=')) == NULL) {
        crinitErrPrint("Config line '%s' has invalid format. Missing delimiter '='.", in);
        return -1;
    }

    out->filename = strndup(in, delim - in);
    out->option = strdup(delim + 1);

    return 0;
}

int crinitConvertConfigArrayToCGroupConfiguration(char **confArray, const int confArraySize,
                                                  crinitCgroupConfiguration_t *result) {
    crinitNullCheck(-1, confArray, result);

    memset(result, 0x00, sizeof(*result));

    result->param = calloc(sizeof(result->param), confArraySize);
    if (result->param == NULL) {
        crinitErrPrint("Failed to allocate memory for config target.");
        return -1;
    }
    result->paramCount = confArraySize;

    for (int i = 0; i < confArraySize; i++) {
        result->param[i] = calloc(sizeof(**(result->param)), 1);
        if (result->param[i] == NULL) {
            crinitErrPrint("Failed to allocate memory for cgroup parameter");
            goto failloop;
        }
        if (crinitCgroupConvertSingleParamToObject(confArray[i], result->param[i]) != 0) {
            crinitErrPrint("Failed to convert single parameter line to object: %s", confArray[i]);
            goto failloop;
        }
    }

    return 0;

failloop:
    for (int i = 0; i < confArraySize; i++) {
        if (result->param[i]) {
            free(result->param[i]->filename);
            free(result->param[i]->option);
            free(result->param[i]);
            result->param[i] = NULL;
        }
    }
    free(result->param);
    result->param = NULL;
    return -1;
}

crinitCgroupConfiguration_t *crinitFindCgroupByName(crinitCgroup_t **cgroups, size_t cgroupsCount, char *name) {
    crinitNullCheck(NULL, cgroups, name);

    for (size_t i = 0; i < cgroupsCount; i++) {
        if (cgroups[i] == NULL) {
            crinitErrPrint("Missmatch between cgroup array and its supplied count.");
            return NULL;
        }
        if (strcmp(cgroups[i]->name, name) == 0) {
            if (cgroups[i]->config == NULL) {
                cgroups[i]->config = calloc(sizeof(*cgroups[i]->config), 1);
            }
            return cgroups[i]->config;
        }
    }

    return NULL;
}

/**
 * @brief Open (and optionally create) a cgroup directory and stores the file descriptor.
 *
 * Opens a file descriptor for the cgroup whose name is stored in @p cgroup.
 * If @p createDir is true and the directory does not exist, it will be created.
 *
 * @param[in,out] cgroup   Pointer to a crinitCgroup_t that holds a valid, non-empty name
 *                         and that receives the open file descriptor.
 *                         Must not be NULL.
 * @param [in] createDir   If true, create the cgroup directory when missing.
 *
 * @return On sucess 0, otherwise -1
 */
static int crinitCgroupOpen(crinitCgroup_t *cgroup, bool createDir) {
    int result = -1;
    crinitNullCheck(result, cgroup);
    crinitNullCheck(result, cgroup->name);
    cgroup->groupFd = -1;
    int cgroupParentFd = -1;

    int cgroupBaseFd = open(CRINIT_CGROUP_PATH, O_DIRECTORY | O_CLOEXEC);
    if (cgroupBaseFd < 0) {
        crinitErrnoPrint("Could not open cgroup base dir.");
    } else {
        int currentFd = -1;
        crinitCgroup_t *cgroupParent = cgroup->parent;
        if (cgroupParent != NULL && cgroupParent->name != NULL) {
            cgroupParentFd = openat(cgroupBaseFd, cgroupParent->name, O_DIRECTORY | O_CLOEXEC);
            if (cgroupParentFd < 0) {
                crinitErrnoPrint("Could not open parent cgroup.");
                goto out;
            }
            currentFd = cgroupParentFd;
        } else {
            currentFd = cgroupBaseFd;
        }

        if (createDir == true) {
            errno = 0;
            if (mkdirat(currentFd, cgroup->name, 0755) == -1 && errno != EEXIST) {
                crinitErrPrint("Could not create cgroup %s.", cgroup->name);
                goto out;
            }
        }

        cgroup->groupFd = openat(currentFd, cgroup->name, O_DIRECTORY | O_CLOEXEC);
        if (cgroup->groupFd < 0) {
            crinitErrnoPrint("Could not open cgroup.");
        } else {
            result = 0;
        }
    }
out:
    if (cgroupBaseFd >= 0) {
        close(cgroupBaseFd);
    }
    if (cgroupParentFd >= 0) {
        close(cgroupParentFd);
    }

    return result;
}

/**
 * @brief Closes a file descriptor of a cgroup directory.
 *
 * Closes a file descriptor for the cgroup who is stored in @p cgroup.
 *
 * @param[in] cgroup   Pointer to a crinitCgroup_t structure that
 *                     holds an open file descriptor to the cgroup directory.
 *                     Must not be NULL.
 */
static void crinitCgroupClose(crinitCgroup_t *cgroup) {
    if (cgroup != NULL) {
        if (cgroup->groupFd >= 0) {
            close(cgroup->groupFd);
            cgroup->groupFd = -1;
        }
    }
}

/**
 * @brief Set a cgroup value to a file within a cgroup directory.
 *
 * Opens the cgroup option file named by @p param->filename relative to the
 * directory file descriptor @p cgroupFd and writes the ASCII value in @p param->option followed by
 * a newline using writev(2). A partial write is treated as an error.
 *
 * @param[in] param         Pointer to a crinitCgroupParam_t that holds a valid, non-empty pathname
 *                          and option.
 *                          Must not be NULL.
 * @param [in] cgroupFd     The open directory file descriptor to the cgroup.
 *
 * @return On sucess 0, otherwise -1
 */
static int crinitCgroupSetParam(crinitCgroupParam_t *param, int cgroupFd) {
    int result = -1;
    crinitNullCheck(result, param);

    int cgroupOptionFd = openat(cgroupFd, param->filename, O_WRONLY | O_CLOEXEC);
    if (cgroupOptionFd < 0) {
        crinitErrnoPrint("Could not open cgroup option.");
    } else {
        struct iovec iov[2] = {{.iov_base = (void *)param->option, .iov_len = strlen(param->option)},
                               {.iov_base = (void *)"\n", .iov_len = 1}};
        size_t written = writev(cgroupOptionFd, iov, 2);
        size_t want = iov[0].iov_len + iov[1].iov_len;
        if (written != want) {
            crinitErrnoPrint("Could not write value %s to option %s.", param->option, param->filename);
        } else {
            result = 0;
        }
    }
    if (cgroupOptionFd >= 0) {
        close(cgroupOptionFd);
    }

    return result;
}

int crinitCGroupConfigure(crinitCgroup_t *cgroup) {
    int result = -1;
    crinitNullCheck(result, cgroup);
    crinitNullCheck(result, cgroup->name, cgroup->config);
    crinitNullCheck(result, cgroup->config->param);

    if (crinitCgroupOpen(cgroup, true) == -1) {
        crinitErrPrint("Could not open cgroup.");
    } else {
        crinitCgroupConfiguration_t *config = cgroup->config;
        result = 0;
        for (size_t index = 0; index < config->paramCount; index++) {
            crinitCgroupParam_t *param = config->param[index];
            if (crinitCgroupSetParam(param, cgroup->groupFd) == -1) {
                result = -1;
                break;
            }
        }
    }
    crinitCgroupClose(cgroup);

    return result;
}

int crinitCGroupAssignPID(crinitCgroup_t *cgroup, pid_t pid) {
    int result = -1;
    crinitNullCheck(result, cgroup);
    crinitNullCheck(result, cgroup->name);
    if (pid <= 0) {
        crinitErrPrint("Invalid process ID.");
        return result;
    }

    if (crinitCgroupOpen(cgroup, false) == -1) {
        crinitErrPrint("Could not open cgroup.");
    } else {
        crinitCgroupParam_t param = {0};
        char buffer[32] = {0};

        param.filename = "cgroup.procs";
        snprintf(buffer, sizeof(buffer), "%d", (int)pid);
        param.option = buffer;

        result = crinitCgroupSetParam(&param, cgroup->groupFd);
    }
    crinitCgroupClose(cgroup);

    return result;
}

int crinitCreateGlobalCGroups(void) {
    crinitGlobOptStore_t *globOpts = crinitGlobOptBorrow();
    if (globOpts == NULL) {
        crinitErrPrint("Could not get exclusive access to global option storage.");
        goto fail;
    }

    crinitCgroup_t *rootCgroup = globOpts->rootCgroup;
    if (globOpts->rootCgroup) {
        if (crinitCGroupConfigure(rootCgroup) != 0) {
            crinitErrPrint("Failed to create crinit root cgroup '%s'.", rootCgroup->name);
            goto fail;
        }
    }

    for (size_t i = 0; i < globOpts->globCgroupsCount; i++) {
        globOpts->globCgroups[i]->parent = rootCgroup;
        if (crinitCGroupConfigure(globOpts->globCgroups[i]) != 0) {
            crinitErrPrint("Failed to create global cgroup '%s'.", globOpts->globCgroups[i]->name);
            goto fail;
        }
    }

    crinitGlobOptRemit();
    return 0;

fail:
    crinitGlobOptRemit();
    return -1;
}

#endif
