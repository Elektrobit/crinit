// SPDX-License-Identifier: MIT
/**
 * @file mock-writev.h
 * @brief Header declaring a mock function for writev().
 */
#ifndef __MOCK_WRITEV_H__
#define __MOCK_WRITEV_H__

#include <sys/uio.h>

/**
 * Mock function for writev().
 *
 * Checks that the right parameters are given and return a preset pointer.
 */
ssize_t __wrap_writev(int fd, const struct iovec *iov,  // NOLINT(readability-identifier-naming)
                      int iovcnt);
// Rationale: Naming scheme fixed due to linker wrapping.

#endif /* __MOCK_WRITEV_H__ */
