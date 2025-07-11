# SPDX-License-Identifier: MIT
find_package(cmocka 1.1.5 REQUIRED)

if(ENABLE_CAPABILITIES)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAP REQUIRED libcap)

    set(CAPABILITIES_SOURCES ${PROJECT_SOURCE_DIR}/src/capabilities.c)

    set(CAPABILITIES_DEFINES
        ENABLE_CAPABILITIES
    )
endif()

function(addFUT)
    cmake_parse_arguments(PARSED_ARGS "" "FUNCTION_NAME;TEST_BINARY_PATH" "" ${ARGN})
    set(fut_file "${PROJECT_BINARY_DIR}/test/functions_under_test.txt")
    
    file(APPEND ${fut_file} "${PARSED_ARGS_FUNCTION_NAME} ${PARSED_ARGS_TEST_BINARY_PATH}\n")
endfunction()

if(ENABLE_CGROUP)
    set(CGROUP_DEFINES
        ENABLE_CGROUP)
endif()

function(create_unit_test)
  cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;INCLUDES;LIBRARIES;DEFINITIONS;WRAPS" ${ARGN})
  get_filename_component(UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.. ABSOLUTE)

  message(STATUS "Create unit test ${PARSED_ARGS_NAME}")
  add_executable(${PARSED_ARGS_NAME} ${PARSED_ARGS_SOURCES})

  target_link_libraries(
    ${PARSED_ARGS_NAME}
    PRIVATE
    ${PARSED_ARGS_LIBRARIES}
    cmocka
    $<IF:$<BOOL:${ENABLE_CAPABILITIES}>,${LIBCAP_LIBRARIES},>
  )

  target_link_options(
    ${PARSED_ARGS_NAME}
    PRIVATE
    ${PARSED_ARGS_WRAPS}
  )

  target_include_directories(
    ${PARSED_ARGS_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/test/utest
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_BINARY_DIR}/inc/
    ${PARSED_ARGS_INCLUDES}
    ${CMOCKA_INCLUDE_DIR}
  )

  target_compile_definitions(
    ${PARSED_ARGS_NAME}
    PRIVATE
    CRINIT_CONFIG_DEFAULT_ELOS_EVENT_POLLING_TIME=${DEFAULT_ELOS_EVENT_POLLING_TIME}
    CRINIT_SOCKFILE="${DEFAULT_CRINIT_SOCKFILE}"
    CRINIT_CONFIG_DEFAULT_SIGKEYDIR="${DEFAULT_SIGKEY_DIR}"
    CRINIT_CONFIG_DEFAULT_INCLDIR="${DEFAULT_INCL_DIR}"
    CRINIT_CONFIG_DEFAULT_TASKDIR="${DEFAULT_TASK_DIR}"
    CRINIT_MACHINE_ID_FILE="${DEFAULT_MACHINE_ID_FILE}"
    ${CAPABILITIES_DEFINES}
    ${CGROUP_DEFINES}
  )

  if(PARSED_ARGS_DEFINITIONS)
    target_compile_definitions(${PARSED_ARGS_NAME} PRIVATE ${PARSED_ARGS_DEFINITIONS})
  endif()

  add_test(NAME ${PARSED_ARGS_NAME} COMMAND ${UNIT_TEST_INTERPRETER} ${PARSED_ARGS_NAME})

  install(TARGETS ${PARSED_ARGS_NAME} DESTINATION ${UNIT_TEST_INSTALL_DIR})
endfunction()

add_subdirectory(mocks)
file(GLOB utests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} utest-*)
foreach(utest ${utests})
    add_subdirectory(${utest})
endforeach()
