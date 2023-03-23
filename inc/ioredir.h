/**
 * @file ioredir.h
 * @brief Header related to IO redirection types and functions.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __IOREDIR_H__
#define __IOREDIR_H__

#include "confparse.h"

/**
 * Type to store an IO redirection definition for a task.
 */
typedef struct ebcl_IoRedir_t {
    int newFd;    ///< The file descriptor to redirect, called 'newFd' equivalent to the man page for dup2()
    int oldFd;    ///< The file descriptor to redirect to, called 'oldFd' equivalent to the man page for dup2()
    char *path;   ///< Path to the file to redirect ebcl_IoRedir_t newFd to, usage of ebcl_IoRedir_t::oldFd and
                  ///< ebcl_IoRedir_t::path should be mutually exclusive to avoid ambiguity.
    int oflags;   ///< The value to use as oflag in the call to open if ebcl_IoRedir_t::path is used, for example
                  ///< O_APPEND.
    mode_t mode;  ///< The value to use as mode in the call to open if ebcl_IoRedir_t::path is used, for example 0644.
} ebcl_IoRedir_t;

/**
 * Initializes an instance of ebcl_IoRedir_t from an IO redirection statement in a task config.
 *
 * Will search the task config for the given key and index. The value must be of the form
 * ```
 * <REDIRECT_FROM> <REDIRECT_TO> [ APPEND | TRUNCATE ] [ OCTAL MODE ]
 * ```
 * Where REDIRECT_FROM is one of STDOUT, STDERR, STDIN and REDIRECT_TO may either also be one of those streams or an
 * absolute path to a file. APPEND or TRUNCATE signify whether an existing file at that location should be appended to
 * or truncated. Default ist TRUNCATE. OCTAL MODE sets the permission bits if the file is newly created. Default is
 * 0644.
 *
 * The function may allocate memory inside the ebcl_IoRedir_t struct which must be freed using EBCL_destroyIoRedir().
 *
 * @param out          The ebcl_IoRedir_t instance to initialize.
 * @param key          The config key to search for in \a in.
 * @param keyArrIndex  The index of the config key.
 * @param in           The config to search in.
 *
 * @return  0 on success, -1 otherwise
 */
int EBCL_initIoRedirFromConfKvList(ebcl_IoRedir_t *out, const char *key, size_t keyArrIndex,
                                   const ebcl_ConfKvList_t *in);

/**
 * Frees memory associated with an initialized instance of ebcl_IoRedir_t.
 *
 * @param ior  The instance whose associated memory shall be freed.
 */
void EBCL_destroyIoRedir(ebcl_IoRedir_t *ior);

/**
 * Initializes an instance of ebcl_IoRedir_t from an already initialized other instance.
 *
 * The resulting instance will contain equal values to the original. If the original instance contains dynamically
 * allocated elements, equivalent memory will be allocated in the new instance and all contents copied.
 *
 * The function may allocate memory inside the ebcl_IoRedir_t struct which must be freed using EBCL_destroyIoRedir().
 *
 * @param dest  The ebcl_IoRedir_t instance to initialize.
 * @param src   The source/original instance to copy data from.
 *
 * @return  0 on success, -1 otherwise
 */
int EBCL_ioRedirCpy(ebcl_IoRedir_t *dest, const ebcl_IoRedir_t *src);

#endif /* __IOREDIR_H__ */
