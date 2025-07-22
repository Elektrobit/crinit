# SPDX-License-Identifier: MIT

if (${ENABLE_CAPABILITIES})
    create_unit_test(
      NAME
        utest-crinit-cap-is-capset-effective
      SOURCES
        utest-crinit-cap-is-capset-effective.c
        case-success.c
        case-unsupported-capability.c
        ${CAPABILITIES_TEST_SOURCES}
        ${PROJECT_SOURCE_DIR}/src/logio.c
        ${CAPABILITIES_SOURCES}
      LIBRARIES
        libmockfunctions
        inih-local
        $<IF:$<BOOL:${ENABLE_CAPABILITIES}>,${LIBCAP_LIBRARIES},>
      WRAPS
        -Wl,--wrap=syscall
    )
endif()
addFUT(FUNCTION_NAME crinitCapIsCapsetEffective TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cap-is-capset-effective")
