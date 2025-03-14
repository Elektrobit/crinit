# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.21)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()
project(crinit VERSION 0.17.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0065 NEW)
include(GNUInstallDirs)

option(ENABLE_SIGNATURE_SUPPORT "Enable signature support using MbedTLS" ON)
option(ENABLE_ELOS "Enable ELOS support" ON)
option(UNIT_TESTS "Build unit tests" ON)
set(UNIT_TEST_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/test/${PROJECT_NAME}/utest" CACHE PATH
  "Directory where the built unit tests will be installed.")
option(API_DOC "Build API documentation" ON)
option(ENABLE_ASAN "Build with -fsanitize=address" OFF)
option(ENABLE_ANALYZER "Build with -fanalyzer (disables -Werror)" OFF)
option(ENABLE_WERROR "Build with -Werror" ON)
set(LIBELOS_SO_FILENAME "" CACHE STRING
    "Name of the libelos shared object crinit will look for at run-time.")
option(MACHINE_ID_EXAMPLE "Build machine-id-gen example application" OFF)
option(INSTALL_EXAMPLE_CONFIGS "Install example configs." OFF)
set(EXAMPLE_TASKDIR "crinit/example" CACHE STRING
  "Task directory to be used in example config, relative to the CMake sysconfdir (usually /etc).")
option(INSTALL_SMOKE_TESTS "Install smoke tests, including necessary configs." OFF)
set(SMOKE_TEST_SCRIPT_DIR "${CMAKE_INSTALL_LIBDIR}/test/crinit/smoketest" CACHE PATH
  "Directory where the smoke test scripts will be installed.")
set(SMOKE_TEST_CONF_DIR "${CMAKE_INSTALL_LIBDIR}/test/crinit/smoketest/config" CACHE PATH
  "Directory where the smoke test config files will be installed.")
option(
  INSTALL_ROBOT_TEST_RESOURCES
  "Install target resources (configs, data, target scripts) for robot integration tests to run."
  OFF
)
set(
  ROBOT_TEST_RESOURCE_DIR
  "${CMAKE_INSTALL_SYSCONFDIR}/crinit/itest"
  CACHE PATH
  "Directory for the robot test target resources to be installed. Note: The robot test scripts assume /etc/crinit/itest"
)
set(
  PWR_SYMLINKS_PATH
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SBINDIR}"
  CACHE PATH
  "Directory where to install symlinks to crinit-ctl for reboot and poweroff. Default: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SBINDIR}"
)
option(INSTALL_BASH_COMPLETION "Install the bash completion script for crinit-ctl." ON)
set(BASH_COMPLETION_DIR "${CMAKE_INSTALL_DATADIR}/bash-completion/completions" CACHE PATH
    "Directory where bash completion scripts should be installed.")

set(CRINIT_LAUNCHER_CMD "\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/crinit-launch\"")

set(DEFAULT_ELOS_EVENT_POLLING_TIME
  500000
  CACHE STRING
  "Default polling interval for ELOS events."
)

set(DEFAULT_CONFIG_SERIES_FILE
  "${CMAKE_INSTALL_SYSCONFDIR}/crinit/default.series"
  CACHE PATH
  "Default series file."
)

set(DEFAULT_CRINIT_SOCKFILE
  "${CMAKE_INSTALL_RUNSTATEDIR}/crinit/crinit.sock"
  CACHE PATH
  "Default path to Crinit's AF_UNIX communication socket."
)

set(DEFAULT_SIGKEY_DIR
  "${CMAKE_INSTALL_SYSCONFDIR}/crinit/pk"
  CACHE PATH
  "Default path to signing keys."
)

set(DEFAULT_INCL_DIR
  "${CMAKE_INSTALL_SYSCONFDIR}/crinit"
  CACHE PATH
  "Default value for INCLUDEDIR global option."
)

set(DEFAULT_TASK_DIR
  "${CMAKE_INSTALL_SYSCONFDIR}/crinit"
  CACHE PATH
  "Default value for TASKDIR global option."
)

set(DEFAULT_MACHINE_ID_FILE
  "${CMAKE_INSTALL_SYSCONFDIR}/machine-id"
  CACHE PATH
  "Default path of the machine id file for the machine ID generator example."
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(ENABLE_ANALYZER)
  set(ENABLE_WERROR OFF)
endif()


set(CRINIT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CRINIT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CRINIT_VERSION_MICRO ${PROJECT_VERSION_PATCH})
set(PKG_VERSION ${CRINIT_VERSION_MAJOR}.${CRINIT_VERSION_MINOR}.${CRINIT_VERSION_MICRO})

find_package(Git)

if(GIT_FOUND)
    macro(FetchGitRevision)
    execute_process(
        COMMAND
        "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE output
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT exit_code EQUAL 0)
        message("failed to retrieve git short commit hash")
    else()
        set(CRINIT_VERSION_GIT ${output})
        message("version ${PKG_VERSION}.${CRINIT_VERSION_GIT}")
    endif()
    endmacro()
    FetchGitRevision()
else()
    set(CRINIT_VERSION_GIT "")
    message("version ${PKG_VERSION}")
endif()

set(CMAKE_C_FLAGS_RELEASE "-O2 -DNODEBUG")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

add_compile_options(
  -Wall -Wextra -Wswitch-enum -pedantic
  $<IF:$<BOOL:${ENABLE_WERROR}>,-Werror,>
  -fstack-protector-strong
  -ffunction-sections -fdata-sections
  -D_DEFAULT_SOURCE
  -D_FORTIFY_SOURCE=2
  $<IF:$<BOOL:${ENABLE_ANALYZER}>,-fanalyzer,>
  $<IF:$<BOOL:${ENABLE_ASAN}>,-fsanitize=address,>
)

add_link_options(
  $<IF:$<BOOL:${ENABLE_ASAN}>,-fsanitize=address,>
)

find_package(RE2C 3 REQUIRED)
if(RE2C_VERSION VERSION_GREATER_EQUAL "4.0.0" AND RE2C_VERSION VERSION_LESS_EQUAL "4.0.2")
    message("re2c version 4.0.0>=4.0.2 detected, activating bit-vector optimization incompatibility workaround.")
    set(RE2C_OPTIONS "-isc" STRING)
else()
    set(RE2C_OPTIONS "-ibsc" STRING)
endif()

add_subdirectory(deps/)
add_subdirectory(src/)
if(UNIT_TESTS)
  enable_testing()
  add_subdirectory(test/utest)
endif(UNIT_TESTS)

if(API_DOC)
  find_package(Doxygen REQUIRED)
  file(GLOB plantuml_images "images/*.plantuml")
  # Exclude rootfs_init_comp as we have already pre-built it as an SVG with a newer PlantUML.
  list(FILTER plantuml_images EXCLUDE REGEX "rootfs_init_comp\.svg$")
  foreach(img ${plantuml_images})
    get_filename_component(output ${img} NAME_WLE)
    add_custom_command(
      OUTPUT
        ${CMAKE_SOURCE_DIR}/doc/api/html/images/${output}.svg
      COMMAND
        plantuml -o ${CMAKE_SOURCE_DIR}/doc/api/html/images -tsvg ${img}
      MAIN_DEPENDENCY
        ${img}
      COMMENT
       "Running plantuml to generate: ${output}"
    )
    list(APPEND doxygen_images ${CMAKE_SOURCE_DIR}/doc/api/html/images/${output}.svg)
  endforeach()
  file(GLOB binary_images "images/*.*")
  list(FILTER binary_images EXCLUDE REGEX "\.plantuml$")
  foreach(img ${binary_images})
    add_custom_command(
      OUTPUT
        ${CMAKE_SOURCE_DIR}/doc/api/html/images/${img}
      COMMAND
        mkdir -p ${CMAKE_SOURCE_DIR}/doc/api/html/images/ && cp ${img} ${CMAKE_SOURCE_DIR}/doc/api/html/images/
      MAIN_DEPENDENCY
        ${img}
      COMMENT
      "Copying binary image: ${img}"
    )
    list(APPEND doxygen_images ${CMAKE_SOURCE_DIR}/doc/api/html/images/${img})
  endforeach()

  add_custom_target(
    api-doc
    ALL
    COMMAND doxygen
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS ${doxygen_images}
  )
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/api/html DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif(API_DOC)

if(INSTALL_EXAMPLE_CONFIGS)
  configure_file(config/example/default.series.in config/example/default.series @ONLY)
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/config/example/"
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${EXAMPLE_TASKDIR}"
    FILES_MATCHING PATTERN "*.crinit"
  )
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/config/example/default.series"
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${EXAMPLE_TASKDIR}"
  )
endif(INSTALL_EXAMPLE_CONFIGS)

if(INSTALL_SMOKE_TESTS)
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/config/test/"
    DESTINATION "${SMOKE_TEST_CONF_DIR}"
    USE_SOURCE_PERMISSIONS
  )
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/test/smoketests/"
    DESTINATION "${SMOKE_TEST_SCRIPT_DIR}"
    PATTERN "*.sh" PERMISSIONS
      OWNER_EXECUTE OWNER_WRITE OWNER_READ
      GROUP_EXECUTE GROUP_READ
      WORLD_EXECUTE WORLD_READ
  )
endif(INSTALL_SMOKE_TESTS)

if(INSTALL_ROBOT_TEST_RESOURCES)
  install(
    PROGRAMS
    "${CMAKE_SOURCE_DIR}/test/integration/scripts/enroll-itest-root-key.sh"
    DESTINATION
    "${CMAKE_INSTALL_BINDIR}"
  )
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/test/integration/service/"
    DESTINATION "${ROBOT_TEST_RESOURCE_DIR}"
    USE_SOURCE_PERMISSIONS
  )
  install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/test/integration/data"
    DESTINATION "${ROBOT_TEST_RESOURCE_DIR}"
    USE_SOURCE_PERMISSIONS
  )
endif(INSTALL_ROBOT_TEST_RESOURCES)

if(INSTALL_BASH_COMPLETION)
  install(
    FILES "${CMAKE_SOURCE_DIR}/completion/crinit-ctl.bash"
    DESTINATION "${BASH_COMPLETION_DIR}"
    RENAME "crinit-ctl"
  )
endif(INSTALL_BASH_COMPLETION)
