# SPDX-License-Identifier: MIT
RE2C_TARGET(NAME lexers_ut_inner INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-lexers-envset-inner
  SOURCES
    utest-lexers-envset-inner.c
    case-success.c
    case-null-input.c
    case-lexer-error.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
addFUT(FUNCTION_NAME crinitEnvVarInnerLex TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-lexers-envset-inner")
