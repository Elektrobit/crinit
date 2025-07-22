# SPDX-License-Identifier: MIT

if (${ENABLE_CGROUP})
    find_package(RE2C 3 REQUIRED)
    RE2C_TARGET(NAME lexers_ut_cgroup_crinit_convert_config_array_to_cgroup_configuration INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

    create_unit_test(
      NAME
        utest-cgroup-crinit-convert-config-array-to-cgroup-configuration
      SOURCES
        utest-cgroup-crinit-convert-config-array-to-cgroup-configuration.c
        case-success.c
        lexers.c
        ${PROJECT_SOURCE_DIR}/src/logio.c
        ${PROJECT_SOURCE_DIR}/src/cgroup.c
        ${PROJECT_SOURCE_DIR}/src/confconv.c
        ${PROJECT_SOURCE_DIR}/src/confparse.c
      LIBRARIES
        libmockfunctions
        inih-local
    )
endif()
