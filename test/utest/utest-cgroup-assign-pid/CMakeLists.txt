# SPDX-License-Identifier: MIT

if (${ENABLE_CGROUP})
  create_unit_test(
    NAME
      utest-cgroup-assign-pid
    SOURCES
      utest-cgroup-assign-pid.c
      case-success.c
      case-wrong-input.c
      case-open-fail.c
      case-openat-fail.c
      case-writev-fail.c
      ${PROJECT_SOURCE_DIR}/src/logio.c
      ${PROJECT_SOURCE_DIR}/src/cgroup.c
    LIBRARIES
      libmockfunctions
    DEFINITIONS
      CRINIT_FSERIES_TESTING
    WRAPS
      -Wl,--wrap=open
      -Wl,--wrap=openat
      -Wl,--wrap=mkdirat
      -Wl,--wrap=close
      -Wl,--wrap=writev
  )
endif()
