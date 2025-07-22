# SPDX-License-Identifier: MIT

if (${ENABLE_CAPABILITIES})
    find_package(RE2C 3 REQUIRED)
    RE2C_TARGET(NAME lexers_ut_task_cap-set-ambient INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

    create_unit_test(
      NAME
        utest-crinit-cap-set-ambient
      SOURCES
        utest-crinit-cap-set-ambient.c
        case-success.c
        case-invalid-capability.c
        ${CAPABILITIES_TEST_SOURCES}
        lexers.c
        ${PROJECT_SOURCE_DIR}/src/logio.c
        ${PROJECT_SOURCE_DIR}/src/confconv.c
        ${PROJECT_SOURCE_DIR}/src/confparse.c
        ${PROJECT_SOURCE_DIR}/src/globopt.c
        ${CAPABILITIES_SOURCES}
      LIBRARIES
        libmockfunctions
        inih-local
        $<IF:$<BOOL:${ENABLE_CAPABILITIES}>,${LIBCAP_LIBRARIES},>
      WRAPS
        -Wl,--wrap=prctl
        -Wl,--wrap=cap_get_bound
    )
endif()
addFUT(FUNCTION_NAME crinitCapSetAmbient TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cap-set-ambient")
