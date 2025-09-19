# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-file-series-stat-filter
  SOURCES
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
    utest-file-series-stat-filter.c
    case-success.c
    case-sisreg-fail.c
    case-fstatat-fail.c
  LIBRARIES
    libmockfunctions
  DEFINITIONS
    CRINIT_FSERIES_TESTING
  WRAPS
    -Wl,--wrap=fstatat
)
addFUT(FUNCTION_NAME crinitStatFilter TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-file-series-stat-filter")
