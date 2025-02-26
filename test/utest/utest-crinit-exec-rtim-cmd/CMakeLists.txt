# SPDX-License-Identifier: MIT
find_package(RE2C 3 REQUIRED)
RE2C_TARGET(NAME lexers_ut_exec-rtim-cmd INPUT ${PROJECT_SOURCE_DIR}/src/lexers.re OUTPUT lexers.c OPTIONS ${RE2C_OPTIONS} -W DEPENDS ${PROJECT_SOURCE_DIR}/inc/lexers.h)
if(ENABLE_ELOS)
    find_package(safu 0.58.2 REQUIRED)
    set(ELOS_SOURCES
        ${PROJECT_SOURCE_DIR}/src/elos-common.c
        ${PROJECT_SOURCE_DIR}/src/elosdep.c
        ${PROJECT_SOURCE_DIR}/src/eloslog.c
    )
    set(ELOS_LIBRARIES
        safu::safu
    )
endif()

create_unit_test(
  NAME
    utest-crinit-exec-rtim-cmd
  SOURCES
    utest-crinit-exec-rtim-cmd.c
    case-shutdown-with-stop-command.c
    lexers.c
    ${PROJECT_SOURCE_DIR}/src/logio.c
    ${PROJECT_SOURCE_DIR}/src/rtimcmd.c
    ${PROJECT_SOURCE_DIR}/src/confconv.c
    ${PROJECT_SOURCE_DIR}/src/confhdl.c
    ${PROJECT_SOURCE_DIR}/src/confmap.c
    ${PROJECT_SOURCE_DIR}/src/confparse.c
    ${PROJECT_SOURCE_DIR}/src/envset.c
    ${PROJECT_SOURCE_DIR}/src/fseries.c
    ${PROJECT_SOURCE_DIR}/src/globopt.c
    ${PROJECT_SOURCE_DIR}/src/ioredir.c
    ${PROJECT_SOURCE_DIR}/src/optfeat.c
    ${PROJECT_SOURCE_DIR}/src/procdip.c
    ${PROJECT_SOURCE_DIR}/src/task.c
    ${PROJECT_SOURCE_DIR}/src/taskdb.c
    ${ELOS_SOURCES}
  LIBRARIES
    libmockfunctions
    inih-local
    ${ELOS_LIBRARIES}
  WRAPS
    -Wl,--wrap=kill
    -Wl,--wrap=mount
    -Wl,--wrap=reboot
    -Wl,--wrap=umount2
)
