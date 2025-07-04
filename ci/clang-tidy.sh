#!/bin/bash -e
# SPDX-License-Identifier: MIT
#
# script to prepare and run clang-tidy on the project
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

if [ $# -gt 1 ]; then
    echo "error: only one build-type allowed"
    exit 1
fi

BUILD_TYPE="${1}"
if [ $# -eq 0 ]; then
    BUILD_TYPE="Debug"
fi

BUILDDIR="${BASEDIR}/build/${BUILD_TYPE}"
CMAKE_BUILD_DIR="${BUILDDIR}/cmake"
RESULTDIR="${BUILDDIR}/result/"

if [ ! -f "${CMAKE_BUILD_DIR}"/compile_commands.json ]; then
    echo "Build environment \"${CMAKE_BUILD_DIR}\" not set up. Please run ci/build.sh first!" >&2
    exit 1
fi

CLANG_TIDY_FLAGS=(-p "${CMAKE_BUILD_DIR}")
# remove -fanalyzer as it's unknown to clang
sed -i -e 's/\-fanalyzer//g' "${CMAKE_BUILD_DIR}"/compile_commands.json

rm -rf $RESULTDIR/clang-tidy
mkdir $RESULTDIR/clang-tidy

cd $BASEDIR


# run clang-tidy for crinit
clang-tidy "${CLANG_TIDY_FLAGS[@]}" -dump-config inc/*.h src/*.c "$CMAKE_BUILD_DIR"/src/crinit-version.c \
    >$RESULTDIR/clang-tidy/config

# catch errors even though we use a pipe to tee
set -o pipefail
clang-tidy "${CLANG_TIDY_FLAGS[@]}" inc/*.h src/*.c "$CMAKE_BUILD_DIR"/src/crinit-version.c 2>&1 \
    | tee $RESULTDIR/clang-tidy/report-crinit

# run clang-tidy for unit tests
for d in "$BASEDIR"/test/utest/utest-*/; do
    SUBDIR="$d"
    clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$SUBDIR"/*.c 2>&1 \
        | tee $RESULTDIR/clang-tidy/report-"$(basename "$SUBDIR")"
done

# run clang-tidy for test-specific headers
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/utest/*.h 2>&1 \
    | tee $RESULTDIR/clang-tidy/report-testheaders

# run clang-tidy for mocks
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/utest/mocks/*.c "$BASEDIR"/test/utest/mocks/*.h 2>&1 \
    | tee $RESULTDIR/clang-tidy/report-mocks
