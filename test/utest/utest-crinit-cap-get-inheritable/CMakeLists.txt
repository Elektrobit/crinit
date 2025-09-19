# SPDX-License-Identifier: MIT

if (${ENABLE_CAPABILITIES})
    create_unit_test(
      NAME
        utest-crinit-cap-get-inheritable
      SOURCES
        utest-crinit-cap-get-inheritable.c
        case-success.c
        case-invalid-capability-pointer.c
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
addFUT(FUNCTION_NAME crinitCapGetInheritable TEST_BINARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/utest-crinit-cap-get-inheritable")
