# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-envset-get
  SOURCES
    utest-envset-get.c
    case-success.c
    case-null-input.c
    case-not-found.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
