find_package(RE2C REQUIRED)
RE2C_TARGET(NAME lexers_ut_inner INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibs -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-lexers-envset-inner
  SOURCES
    utest-lexers-envset-inner.c
    case-success.c
    case-null-input.c
    case-lexer-error.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
  DEFINITIONS
    "-DEBCL_LIB_CONSTRUCTOR=__attribute__((unused))"
    "-DEBCL_LIB_DESTRUCTOR=__attribute__((unused))"
  LIBRARIES
    libmockfunctions
  WRAPS
)

