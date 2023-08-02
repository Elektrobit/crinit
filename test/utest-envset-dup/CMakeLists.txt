# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-envset-dup
  SOURCES
    utest-envset-dup.c
    case-success.c
    case-null-input.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
