// SPDX-License-Identifier: MIT
/**
 * @file mock-malloc.h
 * @brief Header declaring a mock function for malloc().
 */
#ifndef __MOCK_MALLOC_H__
#define __MOCK_MALLOC_H__

#include <stdbool.h>
#include <stddef.h>

/**
 * Mock function for malloc().
 *
 * If crinitMockMallocEnabled is true then it checks that the right parameters are given
 * and return a preset pointer.
 * If crinitMockMallocEnabled is false then the call is forwarded to the genuine free
 * method.
 */
void *__wrap_malloc(size_t size);  // NOLINT(readability-identifier-naming)
                                   // Rationale: Naming scheme fixed due to linker wrapping.

/*
 * Prototype for the genuine malloc function provided by the linker
 */
void *__real_malloc(size_t size);  // NOLINT(readability-identifier-naming)
                                   // Rationale: Naming scheme fixed due to linker wrapping.

/*
 * Define if malloc is used as mock or if malloc forwards to __real_malloc.
 * true - mocking enabled , no real malloc is called
 * false - all calls are forwarded to __real_malloc aka `malloc`
 */
extern bool crinitMockMallocEnabled;
#endif /* __MOCK_MALLOC_H__ */
