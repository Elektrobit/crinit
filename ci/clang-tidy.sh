#!/bin/bash -e
#
# script to prepare and run clang-tidy on the project
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}

rm -rf $BASEDIR/result/clang-tidy
mkdir $BASEDIR/result/clang-tidy
cd $BASEDIR

# generate compile_commands.json for crinit
make clean
rm -f compile_commands.json
CC=clang \
    CFLAGS="-target aarch64-linux-gnu -mcpu=cortex-a53 -march=armv8-a+crc -isystem /usr/aarch64-linux-gnu/include" \
    bear make $(ls src/*.c | sed s/\.c$/.o/g | sed s#src/#obj/#g)

# append tests to compile_commands.json
for d in $BASEDIR/test/utest-*/ ; do
CC=clang \
    CFLAGS="-target aarch64-linux-gnu -mcpu=cortex-a53 -march=armv8-a+crc " \
    CFLAGS+="-isystem /usr/aarch64-linux-gnu/include/ -I$BASEDIR/test/mocks" \
    bear -a make $(ls $d/*.c | sed s/\.c$/.o/g)
done

# append mocks to compile_commands.json
CC=clang \
    CFLAGS="-target aarch64-linux-gnu -mcpu=cortex-a53 -march=armv8-a+crc -isystem /usr/aarch64-linux-gnu/include/" \
    bear -a make $(ls $BASEDIR/test/mocks/*.c | sed s/\.c$/.o/g)

cp compile_commands.json result/clang-tidy/

# run clang-tidy for crinit
clang-tidy -dump-config  -header-filter='inc\/*.h' inc/*.h src/*.c > result/clang-tidy/config
# catch errors even though we use a pipe to tee
set -o pipefail
clang-tidy -header-filter='inc\/*.h' inc/*.h src/*.c 2>&1 | tee result/clang-tidy/report-crinit

# run clang-tidy for unit tests
for d in $BASEDIR/test/utest-*/ ; do
    SUBDIR=$d
    clang-tidy -header-filter='inc\/*.h' $SUBDIR/*.c 2>&1 | tee result/clang-tidy/report-$(basename $SUBDIR)
done

# run clang-tidy for mocks
clang-tidy -header-filter='inc\/*.h' $BASEDIR/test/mocks/*.c 2>&1 | tee result/clang-tidy/report-mocks
