cmake_minimum_required(VERSION 3.21)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()
project(crinit VERSION 0.11.4)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0065 NEW)
include(GNUInstallDirs)

option(UNIT_TESTS "Build unit tests" ON)
option(ENABLE_ASAN "Build with -fsanitize=address" OFF)
option(ENABLE_ANALYZER "Build with -fanalyzer (disables -Werror)" OFF)
option(ENABLE_WERROR "Build with -Werror" ON)
option(MACHINE_ID_EXAMPLE "Build machine-id-gen example application" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(ENABLE_ANALYZER)
  set(ENABLE_WERROR OFF)
endif()

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
  -Wall -Wextra -Wswitch-enum -pedantic
  $<IF:$<BOOL:${ENABLE_WERROR}>,-Werror,>
  -fstack-protector-strong
  -ffunction-sections -fdata-sections
  -D_DEFAULT_SOURCE
  -D_FORTIFY_SOURCE=2
  $<IF:$<BOOL:${ENABLE_ANALYZER}>,-fanalyzer,>
  $<IF:$<BOOL:${ENABLE_ASAN}>,-fsanitize=address,>
)

add_link_options(
  $<IF:$<BOOL:${ENABLE_ASAN}>,-fsanitize=address,>
)

add_subdirectory(deps/)
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
  debbuild
  COMMAND
    mkdir -p ${PROJECT_BINARY_DIR}/debbuild/BUILD ${PROJECT_BINARY_DIR}/debbuild/BUILDROOT ${PROJECT_BINARY_DIR}/debbuild/SPECS ${PROJECT_BINARY_DIR}/debbuild/SOURCES ${PROJECT_BINARY_DIR}/debbuild/SDEBS ${PROJECT_BINARY_DIR}/debbuild/DEBS &&
    cp ${PROJECT_SOURCE_DIR}/packaging/nautilos/crinit.spec ${PROJECT_BINARY_DIR}/debbuild/SPECS/ &&
    tar -czf ${PROJECT_BINARY_DIR}/debbuild/SOURCES/crinit.tar.gz --transform "s,^,crinit/," src/ inc/ test/ config/ deps/ images/ CMakeModules/ CMakeLists.txt Doxyfile README.md &&
    cd ${PROJECT_BINARY_DIR}/debbuild &&
    debbuild
      --define "_topdir ${PROJECT_BINARY_DIR}/debbuild"
      -vv -ba SPECS/crinit.spec
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
