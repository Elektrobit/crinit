// SPDX-License-Identifier: MIT
/**
 * @file cgroup.c
 * @brief Support function for cgroup support
 */

#ifdef ENABLE_CGROUP

#include "cgroup.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "logio.h"

int crinitFreeCgroupParam(crinitCgroupParam *param) {
    crinitNullCheck(-1, param);

    free(param->filename);
    free(param->option);
    param->filename = NULL;
    param->option = NULL;

    return 0;
}

int crinitFreeCgroupConfiguration(crinitCgroupConfiguration *config) {
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

int crinitCopyCgroupParam(crinitCgroupParam *orig, crinitCgroupParam *out) {
    crinitNullCheck(-1, orig, out);

    out->filename = strdup(orig->filename);
    out->option = strdup(orig->option);

    if (out->filename == NULL || out->option == NULL) {
        crinitFreeCgroupParam(out);
        return -1;
    }

    return 0;
}

int crinitCopyCgroupConfiguration(crinitCgroupConfiguration *orig, crinitCgroupConfiguration *out) {
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

int crinitConvertConfigArrayToCGroupConfiguration(char **confArray, const int confArraySize,
                                                  crinitCgroupConfiguration *result) {
    crinitNullCheck(-1, confArray, result);

    memset(result, 0x00, sizeof(*result));

    result->param = calloc(sizeof(result->param), confArraySize);
    if (result->param == NULL) {
        crinitErrPrint("Failed to allocate memory for config target.");
        return -1;
    }
    result->paramCount = confArraySize;

    for (int i = 0; i < confArraySize; i++) {
        char *delim = NULL;
        if ((delim = strchr(confArray[i], '=')) == NULL) {
            crinitErrPrint("Config line '%s' has invalid format. Missing delimiter '='.", confArray[i]);
            goto failloop;
        }
        result->param[i] = calloc(sizeof(**(result->param)), 1);
        if (result->param[i] == NULL) {
            crinitErrPrint("Failed to allocate memory for cgroup parameter");
            goto failloop;
        }
        result->param[i]->filename = strndup(confArray[i], delim - confArray[i]);
        result->param[i]->option = strdup(delim + 1);
    }

    return 0;

failloop:
    if (result->param) {
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
    }
    return -1;
}

#endif
