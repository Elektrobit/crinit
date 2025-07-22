# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-resize-file-series
  SOURCES
    utest-resize-file-series.c
    case-success.c
    case-no-mem-error.c
    case-fse-null-error.c
    case-shrink-zero-error.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=realloc
    -Wl,--wrap=crinitErrPrintFFL
    -Wl,--wrap=crinitErrnoPrintFFL
)
addFUT(FUNCTION_NAME crinitResizeFileSeries TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-resize-file-series")
