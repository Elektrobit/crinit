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
 * Type to specify a global option (set in the series file)
 */
typedef enum {
    CRINIT_GLOBOPT_START,        ///< Marker for beginning of enum, used to calculate number of elements.
    CRINIT_GLOBOPT_DEBUG,        ///< DEBUG global option.
    CRINIT_GLOBOPT_INCLDIR,      ///< INCLUDEDIR global option.
    CRINIT_GLOBOPT_INCL_SUFFIX,  ///< INCLUDE_SUFFIX global option
    CRINIT_GLOBOPT_SHDGRACEP,    ///< SHUTDOWN_GRACE_PERIOD_US global option
    CRINIT_GLOBOPT_USE_SYSLOG,   ///< USE_SYSLOG global option
    CRINIT_GLOBOPT_ENV,          ///< Global task environment variables.
    CRINIT_GLOBOPT_END           ///< Marker for end of enum, used to calculate number of elements.
} crinitGlobOptKey_t;

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
 * Stores a global option value.
 *
 * The data pointed to by \a val is copied to a dynamically allocated storage location. If the global option \a key has
 * been set before, the memory containing the old value is freed. The function uses mutexes internally and is
 * thread-safe.
 *
 * @param key  The global option to set.
 * @param val  Pointer to the data to store for the option.
 * @param sz   Size (in Bytes) of the data to store.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitGlobOptSet(crinitGlobOptKey_t key, const void *val, size_t sz);

/**
 * Retrieves a global option value.
 *
 * If a value for \a key has been stored, it will be written to the location pointed to by \a val. The caller needs to
 * make sure there is enough memory available. The function uses mutexes internally an dis thread-safe. If no value for
 * \a key exists in global option storage, an error will be returned. If a value exists but \a sz is greater than the
 * number of Bytes stored for that value, behavior is undefined.
 *
 * @param key  The global option to get.
 * @param val  Pointer to store the option value.
 * @param sz   The number of Bytes to retrieve.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitGlobOptGet(crinitGlobOptKey_t key, void *val, size_t sz);

/**
 * Function macro to store a bool type option value using crinitGlobOptSet().
 */
#define crinitGlobOptSetBoolean(key, pVal) crinitGlobOptSet(key, pVal, sizeof(bool))
/**
 * Function macro to retrieve a bool type option value using crinitGlobOptGet().
 */
#define crinitGlobOptGetBoolean(key, pVal) crinitGlobOptGet(key, pVal, sizeof(bool))

/**
 * Function macro to store a long int type option value using crinitGlobOptSet().
 */
#define crinitGlobOptSetInteger(key, pVal) crinitGlobOptSet(key, pVal, sizeof(long))
/**
 * Function macro to retrieve a long int type option value crinitGlobOptGet().
 */
#define crinitGlobOptGetInteger(key, pVal) crinitGlobOptGet(key, pVal, sizeof(long))

/**
 * Function macro to store an unsigned long int type option value using crinitGlobOptSet().
 */
#define crinitGlobOptSetUnsigned(key, pVal) crinitGlobOptSet(key, pVal, sizeof(unsigned long))
/**
 * Function macro to retrieve an unsigned long int type option value crinitGlobOptGet().
 */
#define crinitGlobOptGetUnsigned(key, pVal) crinitGlobOptGet(key, pVal, sizeof(unsigned long))

/**
 * Function macro to store an unsigned long long type option value using crinitGlobOptSet().
 */
#define crinitGlobOptSetUnsignedLL(key, pVal) crinitGlobOptSet(key, pVal, sizeof(unsigned long long))
/**
 * Function macro to retrieve an unsigned long long type option value crinitGlobOptGet().
 */
#define crinitGlobOptGetUnsignedLL(key, pVal) crinitGlobOptGet(key, pVal, sizeof(unsigned long long))

/**
 * Stores a string value for a global option.
 *
 * The length of the string is stored as well for later retrieval using crinitGlobOptGetString(). Uses
 * crinitGlobOptSet() and is therefore thread-safe.
 *
 * @param key  The global option to set.
 * @param str  The string value to store, must be null-terminated.
 *
 * @return 0 on success, -1 on error
 */
int crinitGlobOptSetString(crinitGlobOptKey_t key, const char *str);

/**
 * Retrieves a string value stores for a global option.
 *
 * Must be used for a global option value that has been stored using crinitGlobOptSetString(). If this is not the case,
 * behavior is undefined unless the option has not been set, in which case the function will return an error. If
 * successful, the function will write a dynamically allocated string pointer to \a str. The pointer should freed if no
 * longer used. Uses crinitGlobOptGet and is therefore thread-safe.
 *
 * @param key  The global option to get.
 * @param str  Pointer to return a dynamically-allocated copy of the global option value.
 *
 * @return 0 on success, -1 otherwise
 */
int crinitGlobOptGetString(crinitGlobOptKey_t key, char **str);

/**
 * Retrieves an crinitEnvSet_t structure.
 *
 * The instance will be duplicated from the stored one using crinitEnvSetDup(). Memory inside the returned instance will
 * be allocated and should be freed using crinitEnvSetDestroy() when no longer in use.
 *
 * @param es  Return pointer for the retrieved crinitEnvSet_t.
 *
 * @return  0 on success, -1 on error.
 */
int crinitGlobOptGetEnvSet(crinitEnvSet_t *es);
/**
 * Borrows the global environment set from the global option storage array.
 *
 * In this case, borrowing means the caller gets a direct pointer to the crinitEnvSet_t structure in the global option
 * array. After the function has returned successfully, the calling thread will hold the mutex on the global option
 * storage and may freely modify the crinitEnvSet_t at the returned address. Afterwards the calling thread needs to
 * call crinitGlobOptRemitEnvSet() to unlock the option storage mutex or it will cause deadlock.
 *
 * @return  Pointer to the global environment set if successful, NULL otherwise.
 */
crinitEnvSet_t *crinitGlobOptBorrowEnvSet(void);
/**
 * Remits the global environment set to the global option storage.
 *
 * This will unlock the mutex locked by crinitGlobOptBorrowEnvSet(). After this call no further interaction with the
 * global environment set is allowed for the calling thread, unless crinitGlobOptBorrowEnvSet() is called again.
 *
 * @return  0 on success, -1 otherwise.
 */
int crinitGlobOptRemitEnvSet(void);

#endif /* __GLOBOPT_H__ */
