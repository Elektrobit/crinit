// SPDX-License-Identifier: MIT
/**
 * @file mock-strlen.h
 * @brief Header declaring a mock function for strlen().
 */
#ifndef __MOCK_STRLEN_H__
#define __MOCK_STRLEN_H__

#include <stdbool.h>
#include <string.h>

/**
 * Mock function for strlen().
 *
 * If crinitMockStrlenEnabled is true then it checks that the right parameters are
 * given and returns a preset value.
 * If crinitMockStrlenEnabled is false then the call is forwarded to the genuine
 * strlen method.
 */
size_t __wrap_strlen(const char *s);  // NOLINT(readability-identifier-naming)
                                      // Rationale: Naming scheme fixed due to linker wrapping.

/*
 * Prototype for the genuine strlen function provided by the linker
 */
size_t __real_strlen(const char *s);  // NOLINT(readability-identifier-naming)
                                      // Rationale: Naming scheme fixed due to linker wrapping.
/*
 * Define if strlen is used as mock or if strlen forwards to __real_strlen.
 * true - mocking enabled , no real strlen is called
 * false - all calls are forwarded to __real_strlen aka `strlen`
 */
extern bool crinitMockStrlenEnabled;
#endif /* __MOCK_STRLEN_H__ */
