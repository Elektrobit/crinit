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
typedef struct crinitIoRedir_t {
    int newFd;    ///< The file descriptor to redirect, called 'newFd' equivalent to the man page for dup2()
    int oldFd;    ///< The file descriptor to redirect to, called 'oldFd' equivalent to the man page for dup2()
    char *path;   ///< Path to the file to redirect crinitIoRedir_t newFd to, usage of crinitIoRedir_t::oldFd and
                  ///< crinitIoRedir_t::path should be mutually exclusive to avoid ambiguity.
    int oflags;   ///< The value to use as oflag in the call to open if crinitIoRedir_t::path is used, for example
                  ///< O_APPEND.
    mode_t mode;  ///< The value to use as mode in the call to open if crinitIoRedir_t::path is used, for example 0644.
    bool fifo;    ///< If true, the target at crinitIoRedir_t::path is treated as a FIFO special file (named pipe).
} crinitIoRedir_t;

/**
 * Frees memory associated with an initialized instance of crinitIoRedir_t.
 *
 * @param ior  The instance whose associated memory shall be freed.
 */
void crinitDestroyIoRedir(crinitIoRedir_t *ior);

/**
 * Initializes an instance of crinitIoRedir_t from an already initialized other instance.
 *
 * The resulting instance will contain equal values to the original. If the original instance contains dynamically
 * allocated elements, equivalent memory will be allocated in the new instance and all contents copied.
 *
 * The function may allocate memory inside the crinitIoRedir_t struct which must be freed using crinitDestroyIoRedir().
 *
 * @param dest  The crinitIoRedir_t instance to initialize.
 * @param src   The source/original instance to copy data from.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitIoRedirCpy(crinitIoRedir_t *dest, const crinitIoRedir_t *src);

#endif /* __IOREDIR_H__ */
