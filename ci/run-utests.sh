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

UTEST_REPORT="$BASEDIR"/result/"$ARCH"/utest_report.txt
UTEST_LOG="$BASEDIR"/result/"$ARCH"/LastTest.log

# check if ci/build.sh has been run before
if [ ! -d "$BASEDIR/result/$ARCH" ]; then
    echo Build environment not set up. Please run ci/build.sh first!
    exit 1
fi

mkdir -p "$BASEDIR"/result/bin/"$ARCH"/tests
rm -f "$UTEST_REPORT"
rm -f "$UTEST_LOG"
# run tests and copy artifacts
set -o pipefail
RETURNCODE=0
ctest --test-dir "$BASEDIR"/build/"$ARCH" 2>&1 | tee "$UTEST_REPORT" || RETURNCODE=$?

tee "$UTEST_LOG" < "$BASEDIR"/build/"$ARCH"/Testing/Temporary/LastTest.log
exit $RETURNCODE
