// SPDX-License-Identifier: MIT
/**
 * @file envset.h
 * @brief Header defining data types and functions to work with sets of environment variables.
 */
#ifndef __ENVSET_H__
#define __ENVSET_H__

#include <stddef.h>

#include "confparse.h"

/**
 * Default initial allocation size of an environment set.
 */
#define CRINIT_ENVSET_INITIAL_SIZE 128uL
/**
 * Default increment of the allocation size if we run out of space.
 */
#define CRINIT_ENVSET_SIZE_INCREMENT 64uL

/**
 * Data type to hold a mutable environment set.
 */
typedef struct crinitEnvSet_t {
    char **envp;      ///< Array of strings of the form "KEY=value" holding the environment variables. Each string is
                      ///< individually allocated. Terminated by a NULL-pointer after the last initialized element.
    size_t allocSz;   ///< Currently allocated size of envp, may be larger than the number of initialized elements.
    size_t allocInc;  ///< Increment to add to envp's size in case we run out of space.
} crinitEnvSet_t;

/**
 * Initializes an environment set.
 *
 * Will initialize \a es such that crinitEnvSet_t::envp has allocated space for \a initSize pointers to char and
 * crinitEnvSet_t::allocSz, crinitEnvSet_t::allocInc reflect the given parameters.
 *
 * @param es             The environment set to initialize.
 * @param initSize       The allocation size of the initialized environment set.
 * @param sizeIncrement  The increment to add to the allocation size if we run out of space.
 */
int crinitEnvSetInit(crinitEnvSet_t *es, size_t initSize, size_t sizeIncrement);
/**
 * Frees the memory associated with an environment set.
 *
 * Will free all pointers allocated through crinitEnvSetInit() and crinitEnvSetSet().
 *
 * @param es  The environment set to free.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitEnvSetDestroy(crinitEnvSet_t *es);
/**
 * Duplicates an environment set.
 *
 * Will have the same allocation parameter and same content but point to different memory.
 *
 * @param copy  The crinitEnvSet_t to copy to, must be uninitialized or freed via crinitEnvSetDestroy().
 * @param orig  The crinitEnvSet_t to copy from, must have been initialized.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitEnvSetDup(crinitEnvSet_t *copy, const crinitEnvSet_t *orig);
/**
 * Creates a new environment set, given a task/series config and optionally a base set.
 *
 * The new set will eb initialized with the contents of the base set (or empty if NULL) and modified by any `ENV_SET`
 * directives found in the config.
 *
 * @param newSet   The crinitEnvSet_t to create, must be uninitialized or freed via crinitEnvSetDestroy().
 * @param baseSet  The crinitEnvSet_t which serves as a base state, must be initialized or NULL if we "start from
 *                 scratch".
 * @param c        The crinitConfKvList_t which may contain ENV_SET directives to be applied to the new set.
 * @param key      The configuration key to use for reading the environment set.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitEnvSetCreateFromConfKvList(crinitEnvSet_t *newSet, const crinitEnvSet_t *baseSet, const crinitConfKvList_t *c,
                                     const char *restrict key);
/**
 * Gets the value for a given variable from an environment set.
 *
 * @param es  The environment set to search in.
 * @param envName  The name of the variable to look for.
 *
 * @return  A pointer to the variable's value inside the set or NULL if the variable was not found or an error was
 *          encountered.
 */
const char *crinitEnvSetGet(const crinitEnvSet_t *es, const char *envName);
/**
 * Gets the value for a given offset in an environment set.
 *
 * @param es   The environment set to search in.
 * @param idx  The offset to return.
 *
 * @return  A pointer to the variable's value inside the set or NULL if the variable was not found or an error was
 *          encountered.
 */
const char *crinitEnvSetEntry(const crinitEnvSet_t *es, const size_t idx);
/**
 * Sets the value for a given variable in an  environment set.
 *
 * Creates a new variable if no variable with the given name yet exists in the set. Otherwise an existing variable with
 * the same  name will be overwritten.
 *
 * If a new variable is created and the allocation size of the set is not sufficient, the set will be grown by the size
 * increment.
 *
 * @param es       The environment set to be modified, must be initialized.
 * @param envName  The name of the variable to be set.
 * @param envVal   The content of the variable to be set.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitEnvSetSet(crinitEnvSet_t *es, const char *envName, const char *envVal);

#endif /* __ENVSET_H__ */
