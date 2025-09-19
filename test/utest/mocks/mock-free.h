// SPDX-License-Identifier: MIT
/**
 * @file mock-free.h
 * @brief Header declaring a mock function for free().
 */
#ifndef __MOCK_FREE_H__
#define __MOCK_FREE_H__

#include <stdbool.h>

/**
 * Mock function for free().
 *
 * If crinitMockFreeEnabled is true then it checks that the right parameters are
 * given.
 * If crinitMockFreeEnabled is false then the call is forwarded to the genuine free
 * method.
 */
void __wrap_free(void *ptr);  // NOLINT(readability-identifier-naming)
                              // Rationale: Naming scheme fixed due to linker wrapping.
/*
 * Prototype for the genuine free function provided by the linker
 */
void __real_free(void *ptr);  // NOLINT(readability-identifier-naming)
                              // Rationale: Naming scheme fixed due to linker wrapping.
/*
 * Define if free is used as mock or if free forwards to __real_free.
 * true - mocking enabled , no real free is called
 * false - all calls are forwarded to __real_free aka `free`
 */
extern bool crinitMockFreeEnabled;
#endif /* __MOCK_FREE_H__ */
