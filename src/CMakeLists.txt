# SPDX-License-Identifier: MIT
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(RE2C REQUIRED)
find_package(MbedTLS REQUIRED)

configure_file(version.c.in version.c)

RE2C_TARGET(NAME lexers INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS -ibsc -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)

# crinit

add_executable(
  crinit
  crinit.c
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
  sig.c
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
  PRIVATE safu
  PRIVATE ${MBEDTLS_CRYPTO_LIBRARY}
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

# machine-id-gen
if(MACHINE_ID_EXAMPLE)
 add_executable(
    machine-id-gen
    machine-id-gen.c
 )
endif(MACHINE_ID_EXAMPLE)

# Install crinit
install(TARGETS crinit DESTINATION ${CMAKE_INSTALL_SBINDIR})

install(TARGETS crinit-ctl machine-id-gen DESTINATION ${CMAKE_INSTALL_BINDIR})
install(CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/crinit-ctl /${CMAKE_INSTALL_SBINDIR}/poweroff )")
install(CODE "execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/crinit-ctl /${CMAKE_INSTALL_SBINDIR}/reboot )")

install(TARGETS crinit-client
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
