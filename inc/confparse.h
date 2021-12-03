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

/**
 * Linked list to hold key/value pairs read from the config file.
 */
typedef struct ebcl_ConfKvList {
    struct ebcl_ConfKvList *next;  ///< Pointer to next element
    char *key;                     ///< string with "KEY"
    char *val;                     ///< string with "VALUE"
} ebcl_ConfKvList;

/**
 * Parse a config file into a ebcl_ConfKvList.
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
int EBCL_parseConf(ebcl_ConfKvList **confList, const char *filename);

/**
 * Frees memory allocated for an ebcl_ConfKvList by EBCL_parseConf().
 *
 * @param confList Pointer to ebcl_ConfKvList allocated by EBCL_parseConf() and not freed before. If
 *                 confList is NULL, EBCL_freeConfList() will return without freeing any memory.
 */
void EBCL_freeConfList(ebcl_ConfKvList *confList);

/**
 * Get value mapped to key in a ebcl_ConfKvList.
 *
 * Searches for key in \a confList and writes its value's address to \a *val. If key is not found, \a *val
 * is NULL and -1 is returned.
 *
 * @param val  Pointer to a pointer which will point to a string containing the value after execution or NULL if the
 *             value is not found. If \a val is NULL, the function will search for \a key as normal and return
 *             accordingly but nothing will be written to \a *val.
 * @param key  The key to search for.
 * @param c    Pointer to the ebcl_ConfKvList to search in.
 *
 * @return 0 if key is found, -1 otherwise
 *
 */
int EBCL_confListGetVal(char **val, const char *key, const ebcl_ConfKvList *c);
/**
 * Set value mapped to key in a ebcl_ConfKvList.
 *
 * Searches for key in \a confList and replaces its value with val. Memory is de-/allocated as needed. If key is not
 * found, \a *val is NULL and -1 is returned.
 *
 * @param val  The value to write.
 * @param key  The key to search for.
 * @param c    Pointer to the ebcl_ConfKvList to search in.
 *
 * @return 0 if key is found, -1 otherwise
 *
 */
int EBCL_confListSetVal(const char *val, const char *key, ebcl_ConfKvList *c);

/**
 * Extract a boolean value from a ebcl_ConfKvList.
 *
 * Given a ebcl_ConfKvList \a in containing a boolean ("YES"/"NO") key \a key, set \a out to
 * the corresponding truth value ("YES"==true and "NO"==false).
 *
 * @param out  Pointer to a boolean to be set according to the value found.
 * @param key  String with the key to search for in \a in.
 * @param in   Pointer to a ebcl_ConfKvList containing a values for \a key.
 *
 * @return 0 on success, -1 on error.
 */
int EBCL_confListExtractBoolean(bool *out, const char *key, const ebcl_ConfKvList *in);

/**
 * Extract an Array of strings from an ebcl_ConfKvList
 *
 * Will search \a in for \a key and split its value along spaces. Will optionally respect quoting using double quotes
 * if \a double_quoting is set to true. The dynamically-allocated array-of-strings is written to \a *outArgv. If no
 * longer needed it should be freed using EBCL_freeArgvArray(). \a outArgc will contain the number of strings inside \a
 * *outArgv and \a *outArgv will be additionally NULL-terminated, same as argc/argv in main().
 *
 * @param outArgc         will contain the number of strings in outArgv
 * @param outArgv         will contain the value of \a key split along spaces
 * @param key             the key to search for in \a in
 * @param in              the ebcl_ConfKvList to search in
 * @param double_quoting  if true, EBCL_confListExtractArgvArray() will respect quoting with double quotes
 *
 * @return 0 on success, -1 on error
 */
int EBCL_confListExtractArgvArray(int *outArgc, char ***outArgv, const char *key, const ebcl_ConfKvList *in,
                                  bool double_quoting);

/**
 * Free an argv array created by EBCL_confListExtractArgvArray()
 *
 * @param inArgv   The argv array to free, must have been constructed by EBCL_confListExtractArgvArray(). If it is NULL,
 *                 the function will return without freeing any memory.
 */
void EBCL_freeArgvArray(char **inArgv);

#endif /* __CONFPARSE_H__ */
