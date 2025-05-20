# SPDX-License-Identifier: MIT
RE2C_TARGET(NAME lexers_ut_cfgmap INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

create_unit_test(
  NAME
    utest-cfgmap-order
  SOURCES
    utest-cfgmap-order.c
    regression-test.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/confmap.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/task.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/sig.c
    ${PROJECT_SOURCE_DIR}/src/common.c
    ${CAPABILITIES_SOURCES}
LIBRARIES
    libmockfunctions
    inih-local
    ${MBEDTLS_CRYPTO_LIBRARY} 
)
