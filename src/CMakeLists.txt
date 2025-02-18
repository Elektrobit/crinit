# SPDX-License-Identifier: MIT
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(safu 0.58.2 REQUIRED)

configure_file(version.c.in version.c)

# if LIBELOS_SO_FILENAME is unset we do our best to autodetect. If we can't
# we don't set it and let the code use the default value.
if("${LIBELOS_SO_FILENAME}x" STREQUAL "x")
    find_package(libelos QUIET)
    if(libelos_FOUND)
        message("LIBELOS_SO_FILENAME autodetected.")
        add_compile_definitions(LIBELOS_SO_FILENAME="$<TARGET_SONAME_FILE_NAME:elos::libelos>")
    else()
        message("LIBELOS_SO_FILENAME is unset and libelos was not found. Default will be used.")
    endif()
else()
    message("LIBELOS_SO_FILENAME set to ${LIBELOS_SO_FILENAME} via command-line.")
    add_compile_definitions(LIBELOS_SO_FILENAME="${LIBELOS_SO_FILENAME}")
endif()

RE2C_TARGET(NAME lexers INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

# optional compile-time features

if(ENABLE_SIGNATURE_SUPPORT)
    add_compile_definitions(SIGNATURE_SUPPORT)
    set(SIG_SOURCES sig.c)
    find_package(MbedTLS REQUIRED)
endif()

# crinit

add_executable(
  crinit
  crinit.c
  common.c
  confparse.c
  confconv.c
  confhdl.c
  confmap.c
  lexers.c
  elos-common.c
  elosdep.c
  eloslog.c
  envset.c
  kcmdline.c
  task.c
  taskdb.c
  procdip.c
  logio.c
  globopt.c
  minsetup.c
  thrpool.c
  notiserv.c
  rtimcmd.c
  rtimopmap.c
  optfeat.c
  fseries.c
  ioredir.c
  ${SIG_SOURCES}
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

target_include_directories(
  crinit
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)
target_link_libraries(
  crinit
  PRIVATE Threads::Threads
  PRIVATE inih-local
  PRIVATE safu::safu
  PRIVATE ${MBEDTLS_CRYPTO_LIBRARY}
  PRIVATE ${CMAKE_DL_LIBS}
)

target_compile_definitions(
    crinit
    PRIVATE
    CRINIT_LAUNCHER_COMMAND_DEFAULT=${CRINIT_LAUNCHER_CMD}
    CRINIT_CONFIG_DEFAULT_ELOS_EVENT_POLLING_TIME=${DEFAULT_ELOS_EVENT_POLLING_TIME}
    CRINIT_DEFAULT_CONFIG_SERIES="${DEFAULT_CONFIG_SERIES_FILE}"
    CRINIT_SOCKFILE="${DEFAULT_CRINIT_SOCKFILE}"
    CRINIT_CONFIG_DEFAULT_SIGKEYDIR="${DEFAULT_SIGKEY_DIR}"
    CRINIT_CONFIG_DEFAULT_INCLDIR="${DEFAULT_INCL_DIR}"
    CRINIT_CONFIG_DEFAULT_TASKDIR="${DEFAULT_TASK_DIR}"
)

# libcrinit-client

add_library(
  crinit-client SHARED
  crinit-client.c
  envset.c
  logio.c
  rtimcmd.c
  rtimopmap.c
  globopt.c
  sockcom.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

set(CRINIT_PUBLIC_HEADERS
  ../inc/crinit-client.h
  ../inc/crinit-sdefs.h
)

target_compile_definitions(
    crinit-client
    PRIVATE
    CRINIT_CONFIG_DEFAULT_ELOS_EVENT_POLLING_TIME=${DEFAULT_ELOS_EVENT_POLLING_TIME}
    CRINIT_SOCKFILE="${DEFAULT_CRINIT_SOCKFILE}"
    CRINIT_CONFIG_DEFAULT_SIGKEYDIR="${DEFAULT_SIGKEY_DIR}"
    CRINIT_CONFIG_DEFAULT_INCLDIR="${DEFAULT_INCL_DIR}"
    CRINIT_CONFIG_DEFAULT_TASKDIR="${DEFAULT_TASK_DIR}"
)

target_include_directories(
  crinit-client
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)

set_target_properties(crinit-client PROPERTIES
  C_VISIBILITY_PRESET hidden
  VERSION ${PKG_VERSION}
  SOVERSION ${CRINIT_VERSION_MAJOR}
  PUBLIC_HEADER "${CRINIT_PUBLIC_HEADERS}"
)

# crinit-ctl

add_executable(
  crinit-ctl
  crinit-ctl.c
  logio.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

target_include_directories(
  crinit-ctl
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)
target_link_libraries(
  crinit-ctl
  PUBLIC
    crinit-client
    Threads::Threads
)

# crinit-launch

add_executable(
    crinit-launch
    crinit-launch.c
    logio.c
    ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

target_include_directories(
  crinit-launch
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)

target_link_libraries(
  crinit-launch
  PUBLIC
    crinit-client
)

# machine-id-gen
if(MACHINE_ID_EXAMPLE)
  add_executable(
    machine-id-gen
    machine-id-gen.c
  )
endif(MACHINE_ID_EXAMPLE)

# Installation
install(TARGETS crinit DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS crinit-ctl DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS crinit-launch DESTINATION ${CMAKE_INSTALL_BINDIR})

if(MACHINE_ID_EXAMPLE)
  install(TARGETS machine-id-gen DESTINATION ${CMAKE_INSTALL_BINDIR})
endif(MACHINE_ID_EXAMPLE)

install(CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${PWR_SYMLINKS_PATH} )")
install(CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/crinit-ctl ${PWR_SYMLINKS_PATH}/poweroff )")
install(CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/crinit-ctl ${PWR_SYMLINKS_PATH}/reboot )")

install(TARGETS crinit-client
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
