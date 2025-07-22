# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_stop_command_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-cfg-stop_command-handler
  SOURCES
    utest-crinit-cfg-stop_command-handler.c
    case-success.c
    case-null-input.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/task.c
  LIBRARIES
    libmockfunctions
    inih-local
  WRAPS
)
addFUT(FUNCTION_NAME crinitCfgStopCmdHandler TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cfg-stop_command-handler")
