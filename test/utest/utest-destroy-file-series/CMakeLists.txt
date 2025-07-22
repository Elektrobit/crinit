# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-destroy-file-series
  SOURCES
    utest-destroy-file-series.c
    case-success.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=free
    -Wl,--wrap=strdup
)
addFUT(FUNCTION_NAME crinitDestroyFileSeries TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-destroy-file-series")
