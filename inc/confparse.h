/**
 * @file confparse.h
 * @brief Header related to the Config Parser.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CONFPARSE_H__
#define __CONFPARSE_H__

#include <stdbool.h>
#include <sys/types.h>

#include "fseries.h"

#define EBCL_CONFIG_KEYSTR_TASKS "TASKS"
#define EBCL_CONFIG_KEYSTR_SETENV "ENV_SET"
#define EBCL_CONFIG_KEYSTR_SYMLINKS "TASKDIR_FOLLOW_SYMLINKS"
#define EBCL_CONFIG_KEYSTR_TASK_FILE_SUFFIX "TASK_FILE_SUFFIX"
#define EBCL_CONFIG_DEFAULT_TASK_FILE_SUFFIX ".crinit"

/**
 * Linked list to hold key/value pairs read from the config file.
 */
typedef struct ebcl_ConfKvList_t {
    struct ebcl_ConfKvList_t *next;  ///< Pointer to next element
    char *key;                       ///< string with "KEY"
    char *val;                       ///< string with "VALUE"
    size_t keyArrIndex;              ///< Index if this is a key array (e.g. COMMAND[keyArrIndex]), always 0 for
                                     ///< singular keys
} ebcl_ConfKvList_t;

/**
 * Parse a config file into a ebcl_ConfKvList_t.
 *
 * Parses a config file and fills \a confList. Items of \a confList are dynamically allocated
 * (grown) and need to be freed using free_confList(). The format of the config file is expected
 * to be KEY1=VALUE1<newline>KEY2=VALUE2<newline>... Lines beginning with '#' are considered
 * comments.
 *
 * @param confList  will return a pointer to dynamically allocated memory of a ConfKvList filled with the
 *                  key/value-pairs from the config file.
 * @param filename  Path to the configuration file.
 *
 * @return 0 on success, -1 on error
 *
 */
int EBCL_parseConf(ebcl_ConfKvList_t **confList, const char *filename);

/**
 * Frees memory allocated for an ebcl_ConfKvList_t by EBCL_parseConf().
 *
 * @param confList Pointer to ebcl_ConfKvList_t allocated by EBCL_parseConf() and not freed before. If
 *                 confList is NULL, EBCL_freeConfList() will return without freeing any memory.
 */
void EBCL_freeConfList(ebcl_ConfKvList_t *confList);

/**
 * Get value mapped to a key with an index in an ebcl_ConfKvList_t.
 *
 * Searches for \a key with index \a keyArrIndex in \a confList and writes its value's address to \a *val. If a \a key
 * with that index is not found, \a *val is unchanged and -1 is returned.
 *
 * @param val          String return pointer containing the value after execution if it was found.
 * @param key          The key to search for.
 * @param keyArrIndex  The index of the key to search for. The index of a singular (non-array) key is 0.
 * @param c            Pointer to the ebcl_ConfKvList_t to search in.
 *
 * @return 0 if key is found, -1 otherwise
 *
 */
int EBCL_confListGetValWithIdx(char **val, const char *key, size_t keyArrIndex, const ebcl_ConfKvList_t *c);
/**
 * Get a value mapped to a key in an ebcl_confKvList.
 *
 * Maps to a call of EBCL_confListGetValWithIdx() with \a keyArrIndex set to 0. Meant to be used with singular
 * (non-array) keys.
 *
 * See EBCL_confListGetValWithIdx() for further documentation.
 */
#define EBCL_confListGetVal(val, key, c) EBCL_confListGetValWithIdx(val, key, 0, c)

/**
 * Set value mapped to a key with an index in an ebcl_ConfKvList_t.
 *
 * Searches for \a key with index \a keyArrindex in \a confList and replaces its value with \a val. Memory is
 * de-/allocated as needed. If a key with that index is not found, -1 is returned.
 *
 * @param val          The value to write.
 * @param key          The key to search for.
 * @param keyArrIndex  The index of the key to search for. The index of a singular (non-array) key is 0.
 * @param c            Pointer to the ebcl_ConfKvList_t to search in.
 *
 * @return 0 if key is found, -1 otherwise
 *
 */
int EBCL_confListSetValWithIdx(const char *val, const char *key, size_t keyArrIndex, ebcl_ConfKvList_t *c);
/**
 * Set value mapped to a key in an ebcl_confKvList.
 *
 * Maps to a call of EBCL_confListSetValWithIdx() with \a keyArrIndex set to 0. Meant to be used with singular
 * (non-array) keys.
 *
 * See EBCL_confListSetValWithIdx() for further documentation.
 */
#define EBCL_confListSetVal(val, key, c) EBCL_confListSetValWithIdx(val, key, 0, c)

/**
 * Extract a boolean value from a ebcl_ConfKvList_t.
 *
 * Given a ebcl_ConfKvList_t \a in containing a boolean ("YES"/"NO") key \a key, set \a out to
 * the corresponding truth value ("YES"==true and "NO"==false).
 *
 * @param out        Pointer to a boolean to be set according to the value found.
 * @param key        String with the key to search for in \a in.
 * @param mandatory  If true, this function will return an error if \a key is not found in \a in. If false, a
 *                   non-existent \a key will result in successful return and \a out being left untouched.
 * @param in         Pointer to an ebcl_ConfKvList_t to search in.
 *
 * @return 0 on success, -1 on error.
 */
int EBCL_confListExtractBoolean(bool *out, const char *key, bool mandatory, const ebcl_ConfKvList_t *in);

/**
 * Extract an unsigned long long value from an ebcl_ConfKvList.
 *
 * Given an ebcl_ConfKvList_t \a in containing a key \a key with a value representing an unsigned long long (with base
 * \a base), set \a out to the corresponding value.
 *
 * @param out        Pointer to an unsigned long long to be set according to the value found.
 * @param base       Numerical base of the value, e.g. 10 for a decimal number or 16 for hexadecimal.
 * @param key        String with the key to search for in \a in.
 * @param mandatory  If true, this function will return an error if \a key is not found in \a in. If false, a
 *                   non-existent \a key will result in successful return and \a out being left untouched.
 * @param in         Pointer to an ebcl_ConfKvList to search in..
 *
 * @return 0 on success, -1 on error.
 */
int EBCL_confListExtractUnsignedLL(unsigned long long *out, int base, const char *key, bool mandatory,
                                   const ebcl_ConfKvList_t *in);

/**
 * Extract a signed integer value from an ebcl_ConfKvList.
 *
 * Given an ebcl_ConfKvList_t \a in containing a key \a key with a value representing an Integer (with base \a base),
 * set \a out to the corresponding value.
 *
 * @param out        Pointer to an int to be set according to the value found.
 * @param base       Numerical base of the value, e.g. 10 for a decimal number or 16 for hexadecimal.
 * @param key        String with the key to search for in \a in.
 * @param mandatory  If true, this function will return an error if \a key is not found in \a in. If false, a
 *                   non-existent \a key will result in successful return and \a out being left untouched.
 * @param in         Pointer to an ebcl_ConfKvList to search in.
 *
 * @return 0 on success, -1 on error.
 */
int EBCL_confListExtractSignedInt(int *out, int base, const char *key, bool mandatory, const ebcl_ConfKvList_t *in);

/**
 * Extract an array of strings from the value mapped to a key in an ebcl_ConfKvList_t.
 *
 * Maps to a call of EBCL_confExtractArgvArrayWithIdx() with \a keyArrIndex set to 0. Meant to be used with singular
 * (non-array) keys.
 *
 * See EBCL_confListExtractArgvArrayWithIdx() for further documentation.
 */
#define EBCL_confListExtractArgvArray(outArgc, outArgv, key, mandatory, in, doubleQuoting) \
    EBCL_confListExtractArgvArrayWithIdx(outArgc, outArgv, key, 0, mandatory, in, doubleQuoting)
/**
 * Extract an array of strings from the value mapped to an indexed key in an ebcl_ConfKvList_t.
 *
 * Will search \a in for \a key with index \a keyArrIndex and split its value along spaces. Will optionally respect
 * quoting using double quotes if \a doubleQuoting is set to true. The dynamically-allocated array-of-strings is
 * written to \a *outArgv. If no longer needed it should be freed using EBCL_freeArgvArray(). \a outArgc will contain
 * the number of strings inside \a *outArgv and \a *outArgv will be additionally NULL-terminated, same as argc/argv in
 * main().
 *
 * @param outArgc         Will contain the number of strings in outArgv.
 * @param outArgv         Will contain the value of \a key split along spaces.
 * @param key             The key to search for in \a in.
 * @param keyArrIndex     The index of the key to search for. The index of a singular (non-array) key is 0.
 * @param mandatory       If true, this function will return an error if \a key is not found in \a in. If false, a
 *                        non-existent \a key will result in successful return and \a out being left untouched.
 * @param in              The ebcl_ConfKvList_t to search in.
 * @param doubleQuoting   If true, EBCL_confListExtractArgvArray() will respect quoting with double quotes.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_confListExtractArgvArrayWithIdx(int *outArgc, char ***outArgv, const char *key, size_t keyArrIndex,
                                         bool mandatory, const ebcl_ConfKvList_t *in, bool doubleQuoting);

/**
 * Free an argv array created by EBCL_confListExtractArgvArray()/EBCL_confListExtractArgvArrayWithIdx().
 *
 * @param inArgv   The argv array to free, must have been constructed by EBCL_confListExtractArgvArray() or
 *                 EBCL_confListExtractArgvArrayWithIdx(). If it is NULL, the function will return without freeing any
 *                 memory.
 */
void EBCL_freeArgvArray(char **inArgv);

/**
 * Get the maximum index of a key in an ebcl_ConfKvList_t.
 *
 * Keys without a specified index in the config file are parsed as having index 0 (i.e. `COMMAND=...` is equivalent to
 * `COMMAND[0]=...`).
 *
 * @param    c The ebcl_ConfKvList_t to search in.
 * @param  key The key to search for in \a c.
 *
 * @return The key's maximum index within \a c or -1 on error.
 */
ssize_t EBCL_confListKeyGetMaxIdx(const ebcl_ConfKvList_t *c, const char *key);

/**
 * Parse a series file.
 *
 * Will return the task config files to be loaded in \a series. Will also set any global options specified in the series
 * file.
 *
 * @param series     Returns the paths to the task configs specified in the series file.
 * @param filename   The path to the series file to load.
 *
 * @return 0 on success, -1 on failure
 */
int EBCL_loadSeriesConf(ebcl_FileSeries_t *series, const char *filename);

#endif /* __CONFPARSE_H__ */
