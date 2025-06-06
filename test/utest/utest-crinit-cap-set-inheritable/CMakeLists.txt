# SPDX-License-Identifier: MIT

if (${ENABLE_CAPABILITIES})
    create_unit_test(
      NAME
        utest-crinit-cap-set-inheritable
      SOURCES
        utest-crinit-cap-set-inheritable.c
        case-success.c
        case-invalid-capability.c
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
        -Wl,--wrap=cap_set_proc
        -Wl,--wrap=syscall
    )
endif()
