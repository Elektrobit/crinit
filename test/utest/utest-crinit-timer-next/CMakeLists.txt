# SPDX-License-Identifier: MIT

create_unit_test(
  NAME
    utest-crinit-timer-next
  SOURCES
    utest-crinit-timer-next.c
    case-fail.c
    case-thu-jan-1-1970.c
    case-range-success.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/timer.c
  LIBRARIES
    libmockfunctions
    inih-local
  WRAPS
    -Wl,--wrap=getpwuid_r
    -Wl,--wrap=getgrgid_r
)
addFUT(FUNCTION_NAME crinitTimerNext TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-timer-next")
