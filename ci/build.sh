#!/bin/bash
# SPDX-License-Identifier: MIT
###############################################################################
print_info() {
    SCRIPT_NAME="${0##*/}"
    echo "
    Build crinit sources. Specify build type to utilize compiler features for
    release or test build.

    Usage: ${SCRIPT_NAME} [-D<param>] [--no-werror] [-h|--help] [BUILD_TYPE]

    BUILD_TYPE      [Release|Analyzer|Asan|Ubsan], may be empty as well (use
                    standard compile options)
    --clean         remove build directory before build
    --no-werror     disable -Werror flag
    -D<param>       cmake defines to pass to cmake
    -h|--help:      print this help

    Examples:
    ${0} # normal build with no extra compile options (suitable for development iterations)
    ${0} Asan # build with address analyzer enabled
    "
}
###############################################################################
set -euo pipefail

CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}
CMAKE_PARAM=${CMAKE_PARAM:-""}

PARAM=""
OPTION_CLEAN=0
ENABLE_WERROR=ON

while [ $# -gt 0 ]; do
    case ${1} in
        --clean | -c) OPTION_CLEAN=1 ;;
        --help | -h)
            print_info
            exit 0
            ;;
        --no-werror)
            ENABLE_WERROR=OFF
            ;;
        -D)
            CMAKE_PARAM="${CMAKE_PARAM} -D ${2}"
            shift
            ;;
        -D*) CMAKE_PARAM="${CMAKE_PARAM} ${1}" ;;
        -*)
            echo "error: unknown option: ${1}"
            print_info
            exit 1
            ;;
        *)
            PARAM="${PARAM} ${1}"
            ;;
    esac
    shift
done

# shellcheck disable=SC2086 # Intended splitting of PARAM.
set -- $PARAM

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${BASEDIR}/build/${BUILD_TYPE}"
CMAKE_BUILD_DIR="${BUILD_DIR}/cmake"
DIST_DIR="${DIST_DIR:-"${BUILD_DIR}/dist"}"
CMAKE_PARAM="${CMAKE_PARAM} \
    -DCMAKE_INSTALL_PREFIX:PATH=${DIST_DIR}/usr/local/ \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_VERBOSE_MAKEFILE=On \
    -DUNIT_TESTS=On \
    -DMACHINE_ID_EXAMPLE=On \
    -DENABLE_WERROR=${ENABLE_WERROR} \
    -DDEFAULT_CRINIT_SOCKFILE=/run/crinit/crinit.sock \
"

if [ "$OPTION_CLEAN" -eq 1 ]; then
    if [ -e "$BUILD_DIR" ]; then
        echo "Removing $BUILD_DIR ..."
        rm -rf "$BUILD_DIR"
    fi
fi

# shellcheck disable=SC2086 # Intended splitting of CMAKE_PARAM.
cmake -B "$CMAKE_BUILD_DIR" ${CMAKE_PARAM} "$BASEDIR"
make -C "$CMAKE_BUILD_DIR" all install

# copy documentation
cp -a doc "${BUILD_DIR}/"
