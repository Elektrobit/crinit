# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_launch_cmd_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-cfg-launch-cmd-handler
  SOURCES
    utest-crinit-cfg-launch-cmd-handler.c
    case-empty-input.c
    case-invalid-input.c
    case-null-input.c
    case-success.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    $<IF:$<BOOL:${ENABLE_CGROUP}>,${PROJECT_SOURCE_DIR}/src/cgroup.c,>
  LIBRARIES
    libmockfunctions
)
addFUT(FUNCTION_NAME crinitCfgLauncherCmdHandler TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cfg-launch-cmd-handler")
