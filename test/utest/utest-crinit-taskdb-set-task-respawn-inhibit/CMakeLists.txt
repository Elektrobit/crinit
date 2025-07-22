# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_taskdb-set-task-respawn-inhibit INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-taskdb-set-task-respawn-inhibit
  SOURCES
    utest-crinit-taskdb-set-task-respawn-inhibit.c
    case-success.c
    case-failure.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confmap.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/optfeat.c
    ${PROJECT_SOURCE_DIR}/src/task.c
    ${PROJECT_SOURCE_DIR}/src/taskdb.c
    ${CAPABILITIES_SOURCES}
  LIBRARIES
    libmockfunctions
    inih-local
    $<IF:$<BOOL:${ENABLE_CAPABILITIES}>,${LIBCAP_LIBRARIES},>
  WRAPS
    -Wl,--wrap=getpwuid_r
    -Wl,--wrap=getgrgid_r
)
addFUT(FUNCTION_NAME crinitTaskDBSetRespawnInhibit TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-taskdb-set-task-respawn-inhibit")
