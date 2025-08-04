#!/bin/sh -e
# SPDX-License-Identifier: MIT
###############################################################################
print_info() {
    SCRIPT_NAME=${0##*/}
    echo "
    Run the smoketest test suite or parts of it against actual build. Default
    is to run all smoketest tests.

    Usage: ${SCRIPT_NAME} [option] [build type]

    Options:

    -h|--help:                  print this help
    --debug:                    enable debug messages
    --valgrind:                 enabel valgrind during run
    --stop-on-failure:          stop on first failing test
    -i|--interactive:           run a prepared shell environment

    Build Type:

    Debug   - Run smoketest or interactive shell against the Debug Build
    Release - Run smoketest or interactive shell against the Debug Build
    ...     - Any build type located in build/ can be provided

    <Debug> is the default build type, if nothing is specified.

    Examples:

    $>${SCRIPT_NAME} # run all smoke tests on Debug build

    $>${SCRIPT_NAME} Release # run all smoke tests on Release build
    "
}
###############################################################################
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

export SMOKETESTS_VALGRIND=0
export SMOKETESTS_DEBUG=0
export SMOKETESTS_FAILSTOP=0
PARAM=""
while [ $# -gt 0 ]; do
    case ${1} in
        --valgrind)
            SMOKETESTS_VALGRIND=1
            ;;
        --debug)
            SMOKETESTS_DEBUG=1
            ;;
        --stop-on-failure)
            SMOKETESTS_FAILSTOP=1
            ;;
        -h | --help)
            print_info
            exit 0
            ;;
        -*)
            echo "error: unknown option: $1"
            print_info
            exit 1
            ;;
        *)
            PARAM="$PARAM ${1}"
            ;;
    esac
    shift
done
# shellcheck disable=SC2086 # Intended splitting of PARAM.
set -- $PARAM

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${BASEDIR}/build/${BUILD_TYPE}"
CMAKE_BUILD_DIR="${BUILD_DIR}/cmake"
RESULT_DIR="${BUILD_DIR}/result/"

export PREFIX_PATH="${BUILD_DIR}/dist/usr/local"
export BINDIR="${PREFIX_PATH}/bin"
export SBINDIR="${PREFIX_PATH}/bin"
export LIBDIR="${PREFIX_PATH}/lib"
export CONFDIR="${BASEDIR}/config/test"
export LD_LIBRARY_PATH="${LIBDIR}"
export SMOKETEST_RESULTDIR="${SMOKETEST_RESULTDIR-${RESULT_DIR}/smoketest}"

# check if ci/build.sh has been run before
if [ ! -d "$CMAKE_BUILD_DIR" ]; then
    echo Build environment not set up. Please run ci/build.sh for this build type first!
    exit 1
fi

cd "$BASEDIR"

"$BASEDIR"/test/smoketests/smoketests.sh
