#!/bin/bash -e
#
# script to run smoketests for integration
#
# Usage: ci/run-smoketests.sh [Debug]
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)

BUILD_TYPE="Release"
if [ -n "$1" ]; then
    BUILD_TYPE="$1"
    shift
fi

case "$BUILD_TYPE" in
    Release)
        RESULTDIR="$BASEDIR/result/$ARCH"
        ;;
    *)
        RESULTDIR="$BASEDIR/result/$ARCH-$BUILD_TYPE"
        ;;
esac

USE_VALGRIND=0

for ARG in "$@"; do
    case "$ARG" in
        --valgrind)
            USE_VALGRIND=1
            ;;
        *)
            echo "Unknown argument $ARG" >&2
            exit 1
            ;;
    esac
done

BINDIR=$RESULTDIR/bin
LIBDIR=$RESULTDIR/lib
CONFDIR=${BASEDIR}/config/test
export LD_LIBRARY_PATH="${LIBDIR}"

SMOKETESTS_REPORT=$RESULTDIR/smoketests_report.txt
SMOKETESTS_LOG=$RESULTDIR/smoketests.log

# check if ci/build.sh has been run before
if [ ! -d "$RESULTDIR" ]; then
    echo Build environment not set up. Please run ci/build.sh for this build type first!
    exit 1
fi

rm -f "$SMOKETESTS_REPORT"
rm -f "$SMOKETESTS_LOG"

cd "$BASEDIR"

source test/smoketests/lib.sh

source test/smoketests/startstop.sh
