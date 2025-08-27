add_executable(cgroup_demo
  cgroup-demo.c
  ${PROJECT_SOURCE_DIR}/src/cgroup.c
  ${PROJECT_SOURCE_DIR}/src/logio.c
)

target_compile_definitions(cgroup_demo PRIVATE
  ENABLE_CGROUP
)

target_include_directories(cgroup_demo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/inc
)

install(TARGETS cgroup_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
