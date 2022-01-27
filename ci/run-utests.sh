#!/bin/bash -e
#
# script to build and run unit tests
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}
UTEST_REPORT="$BASEDIR"/result/utest_report.txt

# check if ci/build.sh has been run before
if  [ ! -f "/usr/aarch64-linux-gnu/lib/libcmocka.so" ] || \
    [ ! -d "$BASEDIR/result" ]
then
    echo Build environment not set up. Please run ci/build.sh first!
    exit 1
fi

# build tests
make clean
source "$BASEDIR"/ci/cross.env && make tests

mkdir -p "$BASEDIR"/result/bin/aarch64/tests
rm -f "$UTEST_REPORT"
# run tests and copy artifacts
set -o pipefail
TESTERROR=false
while IFS= read -r -d '' t; do
    basename "$t" | tee -a "$UTEST_REPORT"
    cp "$t" "$BASEDIR"/result/bin/aarch64/tests/
    RETURNCODE=0
    qemu-aarch64-static -L /usr/aarch64-linux-gnu "$t" 2>&1 | tee -a "$UTEST_REPORT" || RETURNCODE=$?
    if [ $RETURNCODE != 0 ]; then
        TESTERROR=true
    fi
done < <(find "$BASEDIR"/test -executable -type f -print0)

if [ $TESTERROR = true ]; then
    exit 1
fi
