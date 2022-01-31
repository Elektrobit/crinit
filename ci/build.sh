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
mkdir -p $BASEDIR/build/x86_64
cmake -B $BASEDIR/build/x86_64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=On \
    $BASEDIR
make -C $BASEDIR/build/x86_64
cp $BASEDIR/build/x86_64/src/crinit $BASEDIR/result/bin/x86_64/
cp $BASEDIR/build/x86_64/src/crinit-ctl $BASEDIR/result/bin/x86_64/
cp $BASEDIR/build/x86_64/src/crinit_parsecheck $BASEDIR/result/bin/x86_64/
cp -a $BASEDIR/build/x86_64/src/*.so* $BASEDIR/result/lib/x86_64/

# build and copy aarch64 binaries
mkdir -p $BASEDIR/build/aarch64
cmake -B $BASEDIR/build/aarch64 \
    -DCMAKE_TOOLCHAIN_FILE=$BASEDIR/ci/aarch64-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=On \
    $BASEDIR
make -C $BASEDIR/build/aarch64
cp $BASEDIR/build/aarch64/src/crinit $BASEDIR/result/bin/aarch64/
cp $BASEDIR/build/aarch64/src/crinit-ctl $BASEDIR/result/bin/aarch64/
cp $BASEDIR/build/aarch64/src/crinit_parsecheck $BASEDIR/result/bin/aarch64/
cp -a $BASEDIR/build/aarch64/src/*.so* $BASEDIR/result/lib/aarch64/

# build and copy x86-64 rpm
make -C $BASEDIR/build/x86_64 rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result
cp -a packaging/rpmbuild/SRPMS $BASEDIR/result

# build and copy aarch64 rpm
CMAKE_TOOLCHAIN_FILE=$BASEDIR/ci/aarch64-toolchain.cmake \
    make -C $BASEDIR/build/aarch64 rpmbuild
cp -a packaging/rpmbuild/RPMS $BASEDIR/result

# build and copy documentation
make -C $BASEDIR/build/x86_64 doxygen
cp -a doc $BASEDIR/result

# copy client API headers
cp inc/crinit-client.h $BASEDIR/result/include
cp inc/crinit-sdefs.h $BASEDIR/result/include

