#!/bin/bash -e
#
# project build script
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

# prepare result dir
rm -rf $BASEDIR/result
mkdir -p $BASEDIR/result/bin/x86_64 $BASEDIR/result/bin/aarch64
mkdir -p $BASEDIR/result/lib/x86_64 $BASEDIR/result/lib/aarch64
mkdir -p $BASEDIR/result/include

# build
cd $BASEDIR
# build and copy x86-64 binaries
make clean
make
cp crinit $BASEDIR/result/bin/x86_64/
cp crinit-ctl $BASEDIR/result/bin/x86_64/
cp crinit_parsecheck $BASEDIR/result/bin/x86_64/
cp -a lib/*.so* $BASEDIR/result/lib/x86_64/

# build and copy aarch64 binaries
make clean
CFLAGS="-mcpu=cortex-a53 -march=armv8-a+crc" CC=aarch64-linux-gnu-gcc make
cp crinit $BASEDIR/result/bin/aarch64/
cp crinit-ctl $BASEDIR/result/bin/aarch64/
cp crinit_parsecheck $BASEDIR/result/bin/aarch64/
cp -a lib/*.so* $BASEDIR/result/lib/aarch64/

# build and copy x86-64 rpm
make clean
make rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result
cp -a packaging/rpmbuild/SRPMS $BASEDIR/result

# build and copy aarch64 rpm
make clean
CFLAGS="-mcpu=cortex-a53 -march=armv8-a+crc" CC=aarch64-linux-gnu-gcc make rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result

# build and copy documentation
make doxygen
cp -a doc $BASEDIR/result

# copy client API headers
cp inc/crinit-client.h $BASEDIR/result/include
cp inc/crinit-sdefs.h $BASEDIR/result/include

