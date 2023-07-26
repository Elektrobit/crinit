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

/**
 * Structure definition for the global option storage.
 */
typedef struct crinitGlobOptStore_t {
    bool debug;                    ///< Value for the DEBUG global option.
    bool useSyslog;                ///< Value for the USE_SYSLOG global option.
    char *inclDir;                 ///< Value for the INCLUDEDIR global option.
    char *inclSuffix;              ///< Value for the INCLUDE_SUFFIX global option.
    unsigned long long shdGraceP;  ///< Value for the SHUTDOWN_GRACE_PERIOD_US global option.
    crinitEnvSet_t globEnv;        ///< Storage for global task environment variables.
} crinitGlobOptStore_t;

#define CRINIT_GLOBOPT_DEBUG debug             ///< DEBUG global option
#define CRINIT_GLOBOPT_USE_SYSLOG useSyslog    ///< USE_SYSLOG global option
#define CRINIT_GLOBOPT_INCLDIR inclDir         ///< INCLUDEDIR global option
#define CRINIT_GLOBOPT_INCL_SUFFIX inclSuffix  ///< INCLUDE_SUFFIX global option
#define CRINIT_GLOBOPT_SHDGRACEP shdGraceP     ///< SHUTDOWN_GRACE_PERIOD_US global option
#define CRINIT_GLOBOPT_ENV globEnv             ///< Reference to the global task environment

/** Dummy instance for Generic Selection of members to work (see type-generic macros below). **/
static crinitGlobOptStore_t crinitGenericGlobOptHelper __attribute__((unused));

// clang-format off
// Rationale: Used version of clang-format does not format _Generic macros correctly. This is a known bug and has been
// fixed very recently. We may remove this exemption once we are on the new clang version as standard.
// See: https://github.com/llvm/llvm-project/issues/18080

/**
 * Type-generic macro to get the value of a given global option.
 *
 * Thread-safe as all mapped function are thread-safe.
 *
 * @param globOptMember  One of the member names of crinitGlobOptStore_t (or one of the CRINIT_GLOBOPT_* constants).
 * @param retPtr         Return pointer for value. Type depends on the chosen member of crinitGlobOptStore_t and is
 *                       always `&(member_type)`. In case of `char **` (for a string) and `crinitEnvSet_t *` memory will
 *                       be allocated.
 *
 * @return  0 on success, -1 otherwise
 */
#define crinitGlobOptGet(globOptMember, retPtr) \
    _Generic((crinitGenericGlobOptHelper.globOptMember), \
        char * : crinitGlobOptGetString, \
        bool : crinitGlobOptGetBoolean, \
        unsigned long long : crinitGlobOptGetUnsignedLL, \
        crinitEnvSet_t : crinitGlobOptGetEnvSet) \
        ((offsetof(crinitGlobOptStore_t, globOptMember)), (retPtr))
/**
 * Type-generic macro to get the value of a given global option.
 *
 * Thread-safe as all mapped function are thread-safe.
 *
 * @param globOptMember  One of the member names of crinitGlobOptStore_t (or one of the CRINIT_GLOBOPT_* constants).
 * @param val            Value to store. Type depends on the chosen member of crinitGlobOptStore_t. See function
 *                       signatures of crinitGlobOptGet*().
 *
 * @return  0 on success, -1 otherwise
 */
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

/**
 * Provide direct thread-safe access to the central global option storage.
 *
 * Calling thread will hold an exclusive lock on the central instance of crinitGlobOptStore_t. After the calling thread
 * has finished its operations on the global option storage, it must release the lock using crinitGlobOptRemit().
 *
 * @return  A pointer to the central instance of crinitGlobOptStore_t on success, a NULL pointer if the lock could not
 *          be acquired.
 */
crinitGlobOptStore_t *crinitGlobOptBorrow(void);
/**
 * Release the lock on the global option storage acquired via crinitGlobOptBorrow().
 *
 * @return  0 on success, -1 if the lock could not be released.
 */
int crinitGlobOptRemit(void);

/**
 * Stores a string value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The offset of the member of the global option struct to set.
 * @param val           The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetString(size_t memberOffset, const char *val);
/**
 * Stores a boolean value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           The boolean value to store.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetBoolean(size_t memberOffset, bool val);
/**
 * Stores an unsigned long long value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           The unsigned long long integer value to store.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetUnsignedLL(size_t memberOffset, unsigned long long val);
/**
 * Stores a crinitEnvSet_t value for a global option.
 *
 * Consider using the type-generic macro crinitGlobOptSet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           Pointer to the crinitEnvSet_t which shall be stored.
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
 * Will allocate memory for the returned string. When no longer in use, free() should be called on the returned pointer
 * to free the memory.
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
 * Consider using the type-generic macro crinitGlobOptGet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           Return pointer for the retrieved boolean value.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetBoolean(size_t memberOffset, bool *val);
/**
 * Retrieves an unsigned long long value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           Return pointer for the unsigned long long integer value.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetUnsignedLL(size_t memberOffset, unsigned long long *val);
/**
 * Retrieves a crinitEnvSet_t value from a global option.
 *
 * Consider using the type-generic macro crinitGlobOptGet() which can be used with member names instead of offsets. Will
 * lock the global option storage as long as needed and is thread-safe.
 *
 * @param memberOffset  The global option to set.
 * @param val           Pointer to the crinitEnvSet_t instance to which the global one will be duplicated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptGetEnvSet(size_t memberOffset, crinitEnvSet_t *val);

#endif /* __GLOBOPT_H__ */
