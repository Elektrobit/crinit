# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_task_create_from_conf_kvlist INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

if(ENABLE_CAPABILITIES)
  set(CAPABILITIES_TEST_SOURCES case-invalid-cap-names.c case-invalid-cap-directive.c)
endif()

create_unit_test(
  NAME
    utest-crinit-task-create-from-conf-kvlist
  SOURCES
    utest-crinit-task-create-from-conf-kvlist.c
    case-success.c
    ${CAPABILITIES_TEST_SOURCES}
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    $<IF:$<BOOL:${ENABLE_CGROUP}>,${PROJECT_SOURCE_DIR}/src/cgroup.c,>
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confmap.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/task.c
    ${CAPABILITIES_SOURCES}
  LIBRARIES
    libmockfunctions
    inih-local
    $<IF:$<BOOL:${ENABLE_CAPABILITIES}>,${LIBCAP_LIBRARIES},>
  WRAPS
    -Wl,--wrap=getpwuid_r
    -Wl,--wrap=getgrgid_r
)
addFUT(FUNCTION_NAME crinitTaskCreateFromConfKvList TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-task-create-from-conf-kvlist")
