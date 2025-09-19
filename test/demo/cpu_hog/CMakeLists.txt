add_executable(
  cpu_hog
  cpu_hog.c
)

install(TARGETS cpu_hog DESTINATION ${CMAKE_INSTALL_BINDIR})
