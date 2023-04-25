find_package(RE2C REQUIRED)
RE2C_TARGET(NAME lexers_ut_parse_set INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibsc -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-confconv-envset
  SOURCES
    utest-confconv-envset.c
    case-success.c
    case-null-input.c
    case-wrong-input.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
  LIBRARIES
    libmockfunctions
  WRAPS
)
