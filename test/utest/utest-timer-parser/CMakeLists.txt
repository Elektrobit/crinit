# SPDX-License-Identifier: MIT
RE2C_TARGET(NAME timer_parser_ut_timer_parser INPUT ${PROJECT_SOURCE_DIR}/src/timer_parser.re OUTPUT timer_parser.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/timer.h)

create_unit_test(
  NAME
    utest-timer-parser
  SOURCES
    utest-timer-parser.c
    case-success.c
    case-error.c
    case-null-param.c
    timer_parser.c
    ${PROJECT_SOURCE_DIR}/src/timer.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
addFUT(FUNCTION_NAME crinitTimerParse TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-timer-parser")
