# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_expand_pid_variables INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-expand-pid-variables
  SOURCES
    utest-crinit-expand-pid-variables.c
    case-success.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/procdip.c
    ${PROJECT_SOURCE_DIR}/src/task.c
  LIBRARIES
    libmockfunctions
    inih-local
  WRAPS
)
addFUT(FUNCTION_NAME crinitExpandPIDVariablesInCommands TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-expand-pid-variables")
