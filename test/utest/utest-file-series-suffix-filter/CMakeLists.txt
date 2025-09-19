# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-file-series-suffix-filter
  SOURCES
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
    utest-file-series-suffix-filter.c
    case-success.c
  LIBRARIES
    libmockfunctions
  DEFINITIONS
    CRINIT_FSERIES_TESTING
)
addFUT(FUNCTION_NAME crinitSuffixFilter TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-file-series-suffix-filter")
