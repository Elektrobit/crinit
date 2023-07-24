/**
 * @file globopt.h
 * @brief Header related to global option storage.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __GLOBOPT_H__
#define __GLOBOPT_H__

#include <stdbool.h>
#include <string.h>

#include "envset.h"

typedef struct crinitGlobOptStore_t {
    bool debug;
    bool useSyslog;
    char *inclDir;
    char *inclSuffix;
    unsigned long long shdGraceP;
    crinitEnvSet_t globEnv;
} crinitGlobOptStore_t;

#define CRINIT_GLOBOPT_DEBUG debug
#define CRINIT_GLOBOPT_USE_SYSLOG useSyslog
#define CRINIT_GLOBOPT_INCLDIR inclDir
#define CRINIT_GLOBOPT_INCL_SUFFIX inclSuffix
#define CRINIT_GLOBOPT_SHDGRACEP shdGraceP
#define CRINIT_GLOBOPT_ENV globEnv

static crinitGlobOptStore_t crinitGenericGlobOptHelper __attribute__((unused));

// clang-format off
// Rationale: Used version of clang-format does not format _Generic macros correctly. This is a known bug and has been
// fixed very recently. We may remove this exemption once we are on the new clang version as standard.
// See: https://github.com/llvm/llvm-project/issues/18080
#define crinitGlobOptGet(globOptMember, retPtr) \
    _Generic((crinitGenericGlobOptHelper.globOptMember), \
        char * : crinitGlobOptGetString, \
        bool : crinitGlobOptGetBoolean, \
        unsigned long long : crinitGlobOptGetUnsignedLL, \
        crinitEnvSet_t : crinitGlobOptGetEnvSet) \
        ((offsetof(crinitGlobOptStore_t, globOptMember)), (retPtr))

#define crinitGlobOptSet(globOptMember, val) \
    _Generic((crinitGenericGlobOptHelper.globOptMember), \
        char * : crinitGlobOptSetString, \
        const char * : crinitGlobOptSetString, \
        bool : crinitGlobOptSetBoolean, \
        unsigned long long : crinitGlobOptSetUnsignedLL, \
        crinitEnvSet_t : crinitGlobOptSetEnvSet) \
        ((offsetof(crinitGlobOptStore_t, globOptMember)), (val))
// clang-format on

/**
 * Sets global options to their default values.
 *
 * Uses crinitGlobOptSet() and is therefore thread-safe.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitGlobOptInitDefault(void);

/**
 * Deletes all set global options from storage.
 *
 * Any allocated memory is freed. The function uses mutexes internally and is thread-safe.
 */
void crinitGlobOptDestroy(void);

crinitGlobOptStore_t *crinitGlobOptBorrow(void);
int crinitGlobOptRemit(void);

/**
 * Stores a string value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The offset of the member of the global option struct to set.
 * @param str           The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetString(size_t memberOffset, const char *val);
/**
 * Stores a boolean value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param str           The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetBoolean(size_t memberOffset, bool val);
/**
 * Stores an unsigned long long value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param str  The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetUnsignedLL(size_t memberOffset, unsigned long long val);
/**
 * Stores a crinitEnvSet_t value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param str  The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetEnvSet(size_t memberOffset, const crinitEnvSet_t *val);

/**
 * Retrieves a string value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The offset of the member of the global option struct to set.
 * @param val           Return pointer for the retrieved string. Memory will be allocated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetString(size_t memberOffset, char **val);
/**
 * Retrieves a boolean value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           The string value to retrieve, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetBoolean(size_t memberOffset, bool *val);
/**
 * Retrieves an unsigned long long value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val  The string value to retrieve, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetUnsignedLL(size_t memberOffset, unsigned long long *val);
/**
 * Retrieves a crinitEnvSet_t value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet(). Will lock the global option storage as long as needed and
 * is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val  The string value to retrieve, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetEnvSet(size_t memberOffset, crinitEnvSet_t *val);

#endif /* __GLOBOPT_H__ */
