#!/bin/bash -e
#
# project build script
#
# run with '--no-deps' to avoid rebuilding dependencies after it has been done once.
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

# prepare result dir
rm -rf $BASEDIR/result
mkdir -p $BASEDIR/result/bin/x86_64 $BASEDIR/result/bin/aarch64
mkdir -p $BASEDIR/result/lib/x86_64 $BASEDIR/result/lib/aarch64
mkdir -p $BASEDIR/result/include

# cmocka needs to be installed for unit tests (and their clang-tidy check).
function build_deps {
    # fetch submodules
    git submodule update --init

    # build and install cmocka for aarch64-linux-gnu
    rm -rf $BASEDIR/cmocka/build
    mkdir $BASEDIR/cmocka/build
    cd $BASEDIR/cmocka/build
    cmake \
        -DCMAKE_TOOLCHAIN_FILE=../../ci/aarch64-cmocka-toolchain.cmake \
        -DPICKY_DEVELOPER=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DWITH_EXAMPLES=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr/aarch64-linux-gnu \
        ..
    make
    sudo make install
}

if [ "$1" != "--no-deps" ]; then
    build_deps
fi

# build
cd $BASEDIR
# build and copy x86-64 binaries
make clean
source ci/host-native.env && make
cp crinit $BASEDIR/result/bin/x86_64/
cp crinit-ctl $BASEDIR/result/bin/x86_64/
cp crinit_parsecheck $BASEDIR/result/bin/x86_64/
cp -a lib/*.so* $BASEDIR/result/lib/x86_64/

# build and copy aarch64 binaries
make clean
source ci/cross.env && make
cp crinit $BASEDIR/result/bin/aarch64/
cp crinit-ctl $BASEDIR/result/bin/aarch64/
cp crinit_parsecheck $BASEDIR/result/bin/aarch64/
cp -a lib/*.so* $BASEDIR/result/lib/aarch64/

# build and copy x86-64 rpm
make clean
source ci/host-native.env && make rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result
cp -a packaging/rpmbuild/SRPMS $BASEDIR/result

# build and copy aarch64 rpm
make clean
source ci/cross.env && make rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result

# build and copy documentation
make doxygen
cp -a doc $BASEDIR/result

# copy client API headers
cp inc/crinit-client.h $BASEDIR/result/include
cp inc/crinit-sdefs.h $BASEDIR/result/include

