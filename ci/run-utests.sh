#!/bin/bash -e
# SPDX-License-Identifier: MIT
#
# script to build and run unit tests
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

if [ $# -gt 1 ]; then
    echo "error: only one build-type allowed"
    exit 1
fi

BUILD_TYPE="${1}"
if [ $# -eq 0 ]; then
    BUILD_TYPE="Debug"
fi

BUILDDIR="${BASEDIR}/build/${BUILD_TYPE}"
CMAKE_BUILD_DIR="${BUILDDIR}/cmake"
RESULTDIR="${BUILDDIR}/result/"

UTEST_REPORT="$RESULTDIR"/utest_report.txt
UTEST_LOG="$RESULTDIR"/LastTest.log

# check if ci/build.sh has been run before
if [ ! -d "$RESULTDIR" ]; then
    echo Build environment \"${RESULTDIR}\" not set up. Please run ci/build.sh first!
    exit 1
fi

rm -f "$UTEST_REPORT"
rm -f "$UTEST_LOG"
# run tests and copy artifacts
set -o pipefail
RETURNCODE=0
ctest --test-dir "$CMAKE_BUILD_DIR" 2>&1 | tee "$UTEST_REPORT" || RETURNCODE=$?

tee "$UTEST_LOG" <"$CMAKE_BUILD_DIR"/Testing/Temporary/LastTest.log
exit $RETURNCODE
