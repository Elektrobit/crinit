#!/bin/bash -e
# SPDX-License-Identifier: MIT
#
# project build script
#
# Usage: ./ci/build.sh [Release|Debug]
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)
# architecture name x86_64, aarch64, ...
ARCH_ALT=$(uname -m)

BUILD_TYPE="Release"
if [ -n "$1" ]; then
    BUILD_TYPE="$1"
    shift
fi

case "$BUILD_TYPE" in
    Release)
        BUILDDIR="$BASEDIR/build/$ARCH"
        RESULTDIR="$BASEDIR/result/$ARCH"
        ;;
    *)
        BUILDDIR="$BASEDIR/build/$ARCH-$BUILD_TYPE"
        RESULTDIR="$BASEDIR/result/$ARCH-$BUILD_TYPE"
        ;;
esac

ENABLE_ASAN=OFF
ENABLE_ANALYZER=OFF
ENABLE_WERROR=ON

for ARG in "$@"; do
    case "$ARG" in
        --asan)
            ENABLE_ASAN=ON
            ;;
        --analyzer)
            ENABLE_ANALYZER=ON
            ;;
        --no-werror)
            ENABLE_WERROR=OFF
            ;;
        *)
            echo "Unknown argument $ARG" >&2
            exit 1
            ;;
    esac
done

# prepare result dir
rm -rf "$RESULTDIR"
mkdir -p "$RESULTDIR"/bin
mkdir -p "$RESULTDIR"/lib
mkdir -p "$RESULTDIR"/include

# build
cd "$BASEDIR"

# build binaries
mkdir -p "$BUILDDIR"
cmake -B "$BUILDDIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_VERBOSE_MAKEFILE=On \
    -DUNIT_TESTS=On \
    -DMACHINE_ID_EXAMPLE=On \
    -DENABLE_ASAN="$ENABLE_ASAN" \
    -DENABLE_ANALYZER="$ENABLE_ANALYZER" \
    -DENABLE_WERROR="$ENABLE_WERROR" \
    -DDEFAULT_CRINIT_SOCKFILE=/run/crinit/crinit.sock \
    "$BASEDIR"
make -C "$BUILDDIR"

# copy binaries
cp "$BUILDDIR"/src/crinit "$RESULTDIR"/bin/
cp "$BUILDDIR"/src/crinit-ctl "$RESULTDIR"/bin/
cp "$BUILDDIR"/src/*.so* "$RESULTDIR"/lib/

# copy documentation
cp -a doc "$RESULTDIR"

# copy client API headers
cp inc/crinit-client.h "$RESULTDIR"/include
cp "$BUILDDIR"/inc/crinit-sdefs.h "$RESULTDIR"/include
