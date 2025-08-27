# SPDX-License-Identifier: MIT
function(create_mock_lib)
  cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;INCLUDES;LIBRARIES;DEFINITIONS" ${ARGN})
  message("create ${PARSED_ARGS_NAME} ${PARSED_SOURCES}")

  add_library(${PARSED_ARGS_NAME} STATIC ${PARSED_ARGS_SOURCES})

  if(PARSED_ARGS_DEFINITIONS)
    target_compile_definitions(${PARSED_ARGS_NAME} PRIVATE ${PARSED_ARGS_DEFINITIONS})
  endif()

  target_link_libraries(${PARSED_ARGS_NAME} PRIVATE cmocka ${PARSED_ARGS_LIBRARIES})

  target_include_directories(
    ${PARSED_ARGS_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/test/utest
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_BINARY_DIR}/inc
    INTERFACE
    ${PARSED_ARGS_INCLUDES}
  )
endfunction()

create_mock_lib(
  NAME
    libmockfunctions
  SOURCES
    mock-glob-opt-set-boolean.c
    mock-set-err-stream.c
    mock-set-info-stream.c
    mock-build-rtim-cmd.c
    mock-destroy-rtim-cmd.c
    mock-crinit-xfer.c
    mock-malloc.c
    mock-calloc.c
    mock-close.c
    mock-realloc.c
    mock-free.c
    mock-kill.c
    mock-memset.c
    mock-mkdirat.c
    mock-mount.c
    mock-reboot.c
    mock-strdup.c
    mock-strlen.c
    mock-strcmp.c
    mock-strcpy.c
    mock-stpcpy.c
    mock-open.c
    mock-openat.c
    mock-opendir.c
    mock-closedir.c
    mock-scandir.c
    mock-writev.c
    mock-dirfd.c
    mock-fstatat.c
    mock-err-print.c
    mock-errno-print.c
    mock-envset-init.c
    mock-envset-destroy.c
    mock-umount2.c
    mock-getpwnam_r.c
    mock-getpwuid_r.c
    mock-getgrnam_r.c
    mock-getgrgid_r.c
    mock-prctl.c
    mock-cap_set_proc.c
    mock-cap_get_bound.c
    mock-syscall.c
  INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
)
