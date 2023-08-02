# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-file-series-from-str-arr
  SOURCES
    utest-file-series-from-str-arr.c
    case-success.c
    case-no-mem-error.c
    case-null-param-error.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=strdup
    -Wl,--wrap=crinitErrPrintFFL
    -Wl,--wrap=crinitErrnoPrintFFL
)
