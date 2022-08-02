#!/bin/sh -e
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

if [ "$ARCH" != "amd64" ]; then
    # disable LeakSanitizer as it does not work in qemu-user emulation
    export ASAN_OPTIONS="${ASAN_OPTIONS}:detect_leaks=0"
fi

export SMOKETESTS_VALGRIND=0
export SMOKETESTS_DEBUG=0

for ARG in "$@"; do
    case "$ARG" in
        --valgrind)
            SMOKETESTS_VALGRIND=1
            ;;
        --debug)
            SMOKETESTS_DEBUG=1
            ;;
        *)
            echo "Unknown argument $ARG" >&2
            exit 1
            ;;
    esac
done

export BINDIR=$RESULTDIR/bin
export SBINDIR=$RESULTDIR/bin
export LIBDIR=$RESULTDIR/lib
export CONFDIR=${BASEDIR}/config/test
export LD_LIBRARY_PATH="${LIBDIR}"
export SMOKETESTS_RESULTDIR=$RESULTDIR/smoketests

# check if ci/build.sh has been run before
if [ ! -d "$RESULTDIR" ]; then
    echo Build environment not set up. Please run ci/build.sh for this build type first!
    exit 1
fi

cd "$BASEDIR"

"$BASEDIR"/test/smoketests/smoketests.sh
