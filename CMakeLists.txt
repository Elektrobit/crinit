cmake_minimum_required(VERSION 3.16)
project(crinit)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS NO)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0065 NEW)
include(GNUInstallDirs)

find_package(Git REQUIRED)
set(CRINIT_VERSION_MAJOR 0)
set(CRINIT_VERSION_MINOR 3)
set(CRINIT_VERSION_MICRO 0)
set(PKG_VERSION ${CRINIT_VERSION_MAJOR}.${CRINIT_VERSION_MINOR}.${CRINIT_VERSION_MICRO})

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
    message("failed to retrive git short commit hash")
  else()
    set(CRINIT_VERSION_GIT ${output})
    message("version ${PKG_VERSION}.${CRINIT_VERSION_GIT}")
  endif()
endmacro()
FetchGitRevision()

set(CMAKE_C_FLAGS_RELEASE "-O2 -DNODEBUG")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

add_compile_options(
  -Wall -Wswitch-enum -Werror -pedantic
  -fstack-protector-strong
  -ffunction-sections -fdata-sections
  -D_DEFAULT_SOURCE
  -D_FORTIFY_SOURCE=2
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(src/)

find_package(Doxygen)
add_custom_target(
  doxygen
  COMMAND doxygen
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
