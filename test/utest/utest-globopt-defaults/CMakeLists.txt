# SPDX-License-Identifier: MIT

create_unit_test(
  NAME
    utest-globopt-defaults
  SOURCES
    utest-globopt-defaults.c
    regression-test.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/common.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=calloc
    -Wl,--wrap=strdup
)
# It is necessary to enable struct packing for this test to work, so that we can pre-init the whole struct with a known
# value to track uninitialized fields.
target_compile_options(utest-globopt-defaults PRIVATE -fpack-struct)
addFUT(FUNCTION_NAME crinitGlobalOptBorrow TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-globopt-defaults")
