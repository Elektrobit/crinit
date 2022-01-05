#!/bin/bash -e
#
# script to prepare and run clang-tidy on the project
#
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

rm -rf $BASEDIR/result/clang-tidy
mkdir $BASEDIR/result/clang-tidy
cd $BASEDIR

# generate compile_commands.json
make clean
rm -f compile_commands.json
CC=clang \
    CFLAGS="-target aarch64-linux-gnu -mcpu=cortex-a53 -march=armv8-a+crc -isystem /usr/aarch64-linux-gnu/include" \
    bear make $(ls src/*.c | sed s/\.c$/.o/g | sed s#src/#obj/#g)
cp compile_commands.json result/clang-tidy/

# run clang-tidy
clang-tidy -dump-config  -header-filter='inc\/*.h' inc/*.h src/*.c > result/clang-tidy/config
clang-tidy -header-filter='inc\/*.h' inc/*.h src/*.c 2>&1 | tee result/clang-tidy/report
