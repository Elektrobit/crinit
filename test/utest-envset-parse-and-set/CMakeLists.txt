find_package(RE2C REQUIRED)
RE2C_TARGET(NAME lexers_ut_parse_set INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibs -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-envset-parse-and-set
  SOURCES
    utest-envset-parse-and-set.c
    case-success.c
    case-null-input.c
    case-wrong-input.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
