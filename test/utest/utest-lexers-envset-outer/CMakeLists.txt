# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_outer INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibsc -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-lexers-envset-outer
  SOURCES
    utest-lexers-envset-outer.c
    case-success.c
    case-null-input.c
    case-lexer-error.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
)

