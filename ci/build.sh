#!/bin/bash -e
#
# project build script
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

# build
cd $BASEDIR
make rpmbuild
CFLAGS="-mcpu=cortex-a53 -march=armv8-a+crc" CC=aarch64-linux-gnu-gcc make
make doxygen

# results
rm -rf $BASEDIR/result
mkdir -p $BASEDIR/result
cp -a doc $BASEDIR/result
cp crinit $BASEDIR/result
cp crinit_parsecheck $BASEDIR/result
cp -a packaging/rpmbuild/RPMS $BASEDIR/result
cp -a packaging/rpmbuild/SRPMS $BASEDIR/result

