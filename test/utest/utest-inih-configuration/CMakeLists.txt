# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-inih-configuration
  SOURCES
    utest-inih-configuration.c
    regression-test.c
  LIBRARIES
    inih-local
)
addFUT(FUNCTION_NAME ini_parse_string TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-inih-configuration")
