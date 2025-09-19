# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-init-file-series
  SOURCES
    utest-init-file-series.c
    case-success.c
    case-no-mem-error.c
    case-fse-null-error.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=strdup
    -Wl,--wrap=realloc
    -Wl,--wrap=crinitErrPrintFFL
    -Wl,--wrap=crinitErrnoPrintFFL
)
addFUT(FUNCTION_NAME crinitInitFileSeries TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-init-file-series")
