# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-inih-configuration
  SOURCES
    utest-inih-configuration.c
    regression-test.c
  LIBRARIES
    inih-local
)
