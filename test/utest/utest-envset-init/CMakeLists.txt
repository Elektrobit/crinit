# SPDX-License-Identifier: MIT
create_unit_test(
  NAME
    utest-envset-init
  SOURCES
    utest-envset-init.c
    case-success.c
    case-null-input.c
    case-malloc-error.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=calloc
    -Wl,--wrap=crinitErrPrintFFL
    -Wl,--wrap=crinitErrnoPrintFFL
)

target_compile_options(utest-envset-init PRIVATE "-fno-builtin-malloc") 
