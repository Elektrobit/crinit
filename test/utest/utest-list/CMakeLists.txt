# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-list
  SOURCES
    utest-list.c
    case-success.c
)
addFUT(FUNCTION_NAME crinitListInsert TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-list")
addFUT(FUNCTION_NAME crinitListAppend TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-list")
