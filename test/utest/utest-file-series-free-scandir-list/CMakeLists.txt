# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-file-series-free-scandir-list
  SOURCES
    utest-file-series-free-scandir-list.c
    case-success.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
  LIBRARIES
    libmockfunctions
  DEFINITIONS
    CRINIT_FSERIES_TESTING
  WRAPS
    -Wl,--wrap=free
)
