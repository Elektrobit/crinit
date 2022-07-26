#!/bin/bash -e
#
# script to build and run unit tests
#
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
        BUILDDIR="$BASEDIR/build/$ARCH"
        RESULTDIR="$BASEDIR/result/$ARCH"
        ;;
    *)
        BUILDDIR="$BASEDIR/build/$ARCH-$BUILD_TYPE"
        RESULTDIR="$BASEDIR/result/$ARCH-$BUILD_TYPE"
        ;;
esac

UTEST_REPORT="$RESULTDIR"/utest_report.txt
UTEST_LOG="$RESULTDIR"/LastTest.log

# check if ci/build.sh has been run before
if [ ! -d "$RESULTDIR" ]; then
    echo Build environment not set up. Please run ci/build.sh first!
    exit 1
fi

rm -f "$UTEST_REPORT"
rm -f "$UTEST_LOG"
# run tests and copy artifacts
set -o pipefail
RETURNCODE=0
ctest --test-dir "$BUILDDIR" 2>&1 | tee "$UTEST_REPORT" || RETURNCODE=$?

tee "$UTEST_LOG" < "$BUILDDIR"/Testing/Temporary/LastTest.log
exit $RETURNCODE
