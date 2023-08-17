#!/bin/bash -e
# SPDX-License-Identifier: MIT
#
# project build script
#
# Usage: ./ci/package.sh [Release|Debug]
# Dependency: ci/build.sh needs to be run before
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

# check if ci/build.sh has been run before
if [ ! -d "$RESULTDIR" ]; then
    echo Build environment not set up. Please run ci/build.sh for this build type first!
    exit 1
fi

# prepare result dir
mkdir -p "$RESULTDIR"/deb

# package
cd $BASEDIR

# create debian packages
make -C "$BUILDDIR" debbuild

# copy packages to result dir
cp "$BUILDDIR"/debbuild/SDEBS/*.sdeb "$RESULTDIR"/deb/
cp "$BUILDDIR"/debbuild/DEBS/*/*.deb "$RESULTDIR"/deb/
