create_unit_test(
  NAME
    utest-crinit-set-verbose
  SOURCES
    utest-crinit-set-verbose.c
    utest-crinit-set-verbose-success.c
    utest-crinit-set-verbose-globopt-error.c
    ${PROJECT_SOURCE_DIR}/src/crinit-client.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  DEFINITIONS
    -DEBCL_LIB_CONSTRUCTOR=
    -DEBCL_LIB_DESTRUCTOR=
  LIBRARIES
    libmockfunctions
  WRAPS
    -Wl,--wrap=EBCL_globOptSet
)
# FIXME: -DFOO="" quoting
# FIXME: -Wno-unused-function
target_compile_options(
    utest-crinit-set-verbose
    PRIVATE -Wno-unused-function
)
