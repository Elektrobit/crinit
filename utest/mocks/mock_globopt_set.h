#ifndef __MOCK_GLOBOPT_SET_H__
#define __MOCK_GLOBOPT_SET_H__

#include "globopt.h"

int __wrap_EBCL_globOptSet(ebcl_GlobOptKey_t key, const void *val,  // NOLINT(readability-identifier-naming)
                           size_t sz);                              // Rationale: Naming scheme fixed due to linker
                                                                    // wrapping.

#endif /* __MOCK_GLOBOPT_SET_H__ */
