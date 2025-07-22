# SPDX-License-Identifier: MIT

if (${ENABLE_CAPABILITIES})
    create_unit_test(
      NAME
        utest-crinit-cap-retain-permitted
      SOURCES
        utest-crinit-cap-retain-permitted.c
        case-success.c
        case-fail.c
        ${CAPABILITIES_TEST_SOURCES}
        ${PROJECT_SOURCE_DIR}/src/logio.c
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
addFUT(FUNCTION_NAME crinitCapRetainPermitted TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cap-retain-permitted")
