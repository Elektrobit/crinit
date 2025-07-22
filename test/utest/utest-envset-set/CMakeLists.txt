# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-envset-set
  SOURCES
    utest-envset-set.c
    case-success.c
    case-null-input.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
addFUT(FUNCTION_NAME crinitEnvSetSet TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-envset-set")
