# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-envset-destroy
  SOURCES
    utest-envset-destroy.c
    case-success.c
    case-null-input.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=free
    -Wl,--wrap=crinitErrPrintFFL
)
addFUT(FUNCTION_NAME crinitEnvSetDestroy TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-envset-destroy")
