# SPDX-License-Identifier: MIT
RE2C_TARGET(NAME lexers_ut_kcmdline INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-lexers-kcmdline
  SOURCES
    utest-lexers-kcmdline.c
    case-success.c
    case-null-input.c
    case-lexer-error.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
)

