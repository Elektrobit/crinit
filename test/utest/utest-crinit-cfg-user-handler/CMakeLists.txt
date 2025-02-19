# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_user_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibsc -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-crinit-cfg-user-handler
  SOURCES
    utest-crinit-cfg-user-handler.c
    case-empty-input.c
    case-negative-input.c
    case-null-input.c
    case-success.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=getpwnam_r
    -Wl,--wrap=getpwuid_r
)
