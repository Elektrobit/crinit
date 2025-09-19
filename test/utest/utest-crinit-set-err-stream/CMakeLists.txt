# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-crinit-set-err-stream
  SOURCES
    utest-crinit-set-err-stream.c
    case-set.c
    ${PROJECT_SOURCE_DIR}/src/crinit-client.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  DEFINITIONS
    "-DCRINIT_LIB_CONSTRUCTOR=__attribute__((unused))"
    "-DCRINIT_LIB_DESTRUCTOR=__attribute__((unused))"
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=crinitSetErrStream
)
addFUT(FUNCTION_NAME crinitClientSetErrStream TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-set-err-stream")
