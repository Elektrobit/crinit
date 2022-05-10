cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()
project(crinit VERSION 0.5.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS NO)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0065 NEW)
include(GNUInstallDirs)

option(UNIT_TESTS "Build unit tests" ON)

find_package(Git REQUIRED)
set(CRINIT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CRINIT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CRINIT_VERSION_MICRO ${PROJECT_VERSION_PATCH})
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
    message("failed to retrieve git short commit hash")
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
  -Wall -Wextra -Wswitch-enum -Werror -pedantic
  -fstack-protector-strong
  -ffunction-sections -fdata-sections
  -D_DEFAULT_SOURCE
  -D_FORTIFY_SOURCE=2
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

add_subdirectory(src/)
if(UNIT_TESTS)
  enable_testing()
  add_subdirectory(test/)
endif(UNIT_TESTS)

find_package(Doxygen)
file(GLOB images "images/*.plantuml")
foreach(img ${images})
    get_filename_component(output ${img} NAME_WLE)
    add_custom_command(
      OUTPUT
        ${CMAKE_SOURCE_DIR}/doc/html/images/${output}.svg
      COMMAND
        plantuml -o ${CMAKE_SOURCE_DIR}/doc/html/images -tsvg ${img}
      MAIN_DEPENDENCY
        ${img}
      COMMENT
       "Running plantuml to generate: ${output}"
    )
    list(APPEND doxygen_images ${CMAKE_SOURCE_DIR}/doc/html/images/${output}.svg)
endforeach()
add_custom_target(
  doxygen
  COMMAND doxygen
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS ${doxygen_images}
)

add_custom_target(
  rpmbuild
  COMMAND
    mkdir -p packaging/rpmbuild/SOURCES &&
    tar -czf packaging/rpmbuild/SOURCES/crinit-${PKG_VERSION}.tar.gz --transform "s,^,crinit-${PKG_VERSION}/," src/ inc/ test/ config/ images/ CMakeLists.txt Doxyfile README.md &&
    cd packaging/rpmbuild &&
    rpmbuild
      --target ${CMAKE_SYSTEM_PROCESSOR}
      --nodeps
      --define "_topdir ${PROJECT_SOURCE_DIR}/packaging/rpmbuild"
      -v -ba SPECS/crinit-git.spec
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
