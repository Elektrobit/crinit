# SPDX-License-Identifier: MIT

if (${ENABLE_CGROUP})
    find_package(RE2C 3 REQUIRED)
    RE2C_TARGET(NAME lexers_ut_cgroup_name_handler INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

    create_unit_test(
      NAME
        utest-cgroup-crinit-cfg-cgroup-name-handler
      SOURCES
        utest-cgroup-crinit-cfg-cgroup-name-handler.c
        case-empty-input.c
        case-null-input.c
        case-success.c
        lexers.c
        ${PROJECT_SOURCE_DIR}/src/logio.c
        ${PROJECT_SOURCE_DIR}/src/confconv.c
        ${PROJECT_SOURCE_DIR}/src/confhdl.c
        ${PROJECT_SOURCE_DIR}/src/confparse.c
        ${PROJECT_SOURCE_DIR}/src/cgroup.c
        ${PROJECT_SOURCE_DIR}/src/task.c
        ${PROJECT_SOURCE_DIR}/src/ioredir.c
        ${PROJECT_SOURCE_DIR}/src/envset.c
        ${PROJECT_SOURCE_DIR}/src/globopt.c
      LIBRARIES
        libmockfunctions
        inih-local
    )
endif()
