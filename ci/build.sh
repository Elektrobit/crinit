#!/bin/bash -e
#
# project build script
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)
# architecture name x86_64, aarch64, ...
ARCH_ALT=$(uname -m)

# prepare result dir
rm -rf $BASEDIR/result/$ARCH
mkdir -p $BASEDIR/result/$ARCH/bin
mkdir -p $BASEDIR/result/$ARCH/lib
mkdir -p $BASEDIR/result/$ARCH/include
mkdir -p $BASEDIR/result/$ARCH/rpm

# build
cd $BASEDIR

# build binaries
mkdir -p $BASEDIR/build/$ARCH
cmake -B $BASEDIR/build/$ARCH \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=On \
    -DUNIT_TESTS=On \
    $BASEDIR
make -C $BASEDIR/build/$ARCH

# copy binaries
cp $BASEDIR/build/$ARCH/src/crinit $BASEDIR/result/$ARCH/bin/
cp $BASEDIR/build/$ARCH/src/crinit-ctl $BASEDIR/result/$ARCH/bin/
cp $BASEDIR/build/$ARCH/src/crinit_parsecheck $BASEDIR/result/$ARCH/bin/
cp $BASEDIR/build/$ARCH/src/*.so* $BASEDIR/result/$ARCH/lib/

# build and copy rpm
rm -rf packaging/rpmbuild/RPMS/$ARCH_ALT
rm -rf packaging/rpmbuild/SRPMS
make -C $BASEDIR/build/$ARCH rpmbuild
cp packaging/rpmbuild/RPMS/$ARCH_ALT/* $BASEDIR/result/$ARCH/rpm/
cp packaging/rpmbuild/SRPMS/* $BASEDIR/result/$ARCH/rpm/

# build and copy documentation
make -C $BASEDIR/build/$ARCH doxygen
cp -a doc $BASEDIR/result

# copy client API headers
cp inc/crinit-client.h $BASEDIR/result/$ARCH/include
cp inc/crinit-sdefs.h $BASEDIR/result/$ARCH/include
