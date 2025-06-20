add_executable(
  memory_hog
  memory_hog.c
)

install(TARGETS memory_hog DESTINATION ${CMAKE_INSTALL_BINDIR})
