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
addFUT(FUNCTION_NAME crinitEnvSetDup TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-envset-dup")
