# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_elos-poll-interval_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-cfg-elos-poll-interval-handler
  SOURCES
    utest-crinit-cfg-elos-poll-interval-handler.c
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
  LIBRARIES
    libmockfunctions
)
addFUT(FUNCTION_NAME crinitCfgElosEventPollIntervalHandler TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cfg-elos-poll-interval-handler")
