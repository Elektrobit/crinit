# SPDX-License-Identifier: MIT

if(${ENABLE_CAPABILITIES})
    find_package(RE2C 3 REQUIRED)
    RE2C_TARGET(NAME lexers_ut_task_cap-convert-to-bitmask INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

    create_unit_test(
      NAME
        utest-crinit-cap-convert-to-bitmask
      SOURCES
        utest-crinit-cap-convert-to-bitmask.c
        case-success.c
        case-invalid-capability-name.c
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
    )
endif()
addFUT(FUNCTION_NAME crinitCapConvertToBitmask TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cap-convert-to-bitmask")
