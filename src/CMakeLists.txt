set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

configure_file(version.c.in version.c)

# crinit

add_executable(
  crinit
  crinit.c
  confparse.c
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
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

target_include_directories(
  crinit
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)
target_link_libraries(
  crinit
  PRIVATE Threads::Threads
)


# libcrinit-client

add_library(
  crinit-client SHARED
  crinit-client.c
  logio.c
  rtimcmd.c
  rtimopmap.c
  globopt.c
  sockcom.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
)

target_include_directories(
  crinit-client
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)

set_target_properties(crinit-client PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(crinit-client PROPERTIES VERSION ${PKG_VERSION})
set_target_properties(crinit-client PROPERTIES SOVERSION ${CRINIT_VERSION_MAJOR})


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


# crinit_parsecheck

add_executable(
  crinit_parsecheck
  crinit_parsecheck.c
  confparse.c
  taskdb.c
  logio.c
)
target_include_directories(
  crinit_parsecheck
  PRIVATE ${PROJECT_SOURCE_DIR}/inc/
)
target_link_libraries(
  crinit_parsecheck
  PRIVATE Threads::Threads
)

# machine-id-gen
if(MACHINE_ID_EXAMPLE)
 add_executable(
    machine-id-gen
    machine-id-gen.c
 )
endif(MACHINE_ID_EXAMPLE)

# install

install(TARGETS crinit crinit-ctl crinit_parsecheck machine-id-gen DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS crinit-client DESTINATION ${CMAKE_INSTALL_LIBDIR})
