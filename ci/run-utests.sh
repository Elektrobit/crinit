#!/bin/bash -e
#
# script to build and run unit tests
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

# check if ci/build.sh has been run before
if  [ ! -f "/usr/aarch64-linux-gnu/lib/libcmocka.so" ] || \
    [ ! -d "$BASEDIR/result" ]
then
    echo Build environment not set up. Please run ci/build.sh first!
    exit 1
fi

# build tests
make clean
source $BASEDIR/ci/cross.env && make tests

mkdir -p $BASEDIR/result/bin/aarch64/tests
# run tests and copy artifacts
find $BASEDIR/test -executable -type f -exec sh -c \
    "echo \$(basename {}); cp {} $BASEDIR/result/bin/aarch64/tests/; qemu-aarch64-static -L /usr/aarch64-linux-gnu {}" \; 2>&1 \
    | tee -a $BASEDIR/result/utest_report.txt
