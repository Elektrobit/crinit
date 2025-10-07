# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_dep_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-cfg-dep-handler
  SOURCES
    utest-crinit-cfg-dep-handler.c
    case-empty-input.c
    case-invalid-value.c
    case-null-input.c
    case-success.c
    case-err-config-type.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/task.c
    $<IF:$<BOOL:${ENABLE_CGROUP}>,${PROJECT_SOURCE_DIR}/src/cgroup.c,>
  LIBRARIES
    libmockfunctions
    inih-local
)
addFUT(FUNCTION_NAME crinitCfgDepHandler TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cfg-dep-handler")
