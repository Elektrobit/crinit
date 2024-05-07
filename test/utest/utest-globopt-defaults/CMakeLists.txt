# SPDX-License-Identifier: MIT

create_unit_test(
  NAME
    utest-globopt-defaults
  SOURCES
    utest-globopt-defaults.c
    regression-test.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/common.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=calloc
    -Wl,--wrap=memset
    -Wl,--wrap=strdup
)
# This helps by packing structs and making memset a mock-able function call instead of an inline loop. _FORTIFY_SOURCE
# is unfortunately incompatible with -O0. This has only an effect on the test code as CMake recompiles all test sources
# in a different folder.
target_compile_options(utest-globopt-defaults PRIVATE -fno-builtin -fpack-struct -O0)
get_target_property(defs utest-globopt-defaults COMPILE_OPTIONS)
list(FILTER defs EXCLUDE REGEX [[_FORTIFY_SOURCE]])
set_property(TARGET utest-globopt-defaults PROPERTY COMPILE_OPTIONS ${defs})
