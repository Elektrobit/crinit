/**
 * @file envset.h
 * @brief Header defining data types and functions to work with sets of environment variables.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __ENVSET_H__
#define __ENVSET_H__

#include <stddef.h>

#include "confparse.h"

#define EBCL_ENVSET_INITIAL_SIZE 128uL
#define EBCL_ENVSET_SIZE_INCREMENT 64uL

typedef struct ebcl_EnvSet_t {
    char **envp;
    size_t allocSz;
    size_t allocInc;
} ebcl_EnvSet_t;

int EBCL_envSetInit(ebcl_EnvSet_t *es, size_t initSize, size_t sizeIncrement);
int EBCL_envSetDestroy(ebcl_EnvSet_t *es);
int EBCL_envSetDup(ebcl_EnvSet_t *copy, const ebcl_EnvSet_t *orig);
int EBCL_envSetCreateFromConfKvList(ebcl_EnvSet_t *newSet, const ebcl_EnvSet_t *baseSet, const ebcl_ConfKvList_t *c);

const char *EBCL_envSetGet(const ebcl_EnvSet_t *es, const char *envName);
int EBCL_envSetSet(ebcl_EnvSet_t *es, const char *envName, const char *envVal);
int EBCL_envSetParseAndSet(ebcl_EnvSet_t *es, const char *envConf);

#endif /* __ENVSET_H__ */
