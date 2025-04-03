#!/bin/bash -e
# SPDX-License-Identifier: MIT
#
# script to prepare and run clang-tidy on the project
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}
# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)
BUILDDIR=build/"$ARCH"

if [ ! -f "${BUILDDIR}"/compile_commands.json ]; then
    echo "Build environment not set up. Please run ci/build.sh first!" >&2
    exit 1
fi

rm -rf "$BASEDIR"/result/clang-tidy
mkdir "$BASEDIR"/result/clang-tidy
cd "$BASEDIR"

CLANG_TIDY_FLAGS=(-p "${BUILDDIR}")

# run clang-tidy for crinit
clang-tidy "${CLANG_TIDY_FLAGS[@]}" -dump-config "$BASEDIR"/inc/*.h "$BASEDIR"/src/*.c "$BASEDIR"/"$BUILDDIR"/src/crinit-version.c \
    >"$BASEDIR"/result/clang-tidy/config
# catch errors even though we use a pipe to tee
set -o pipefail
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/inc/*.h "$BASEDIR"/src/*.c "$BASEDIR"/"$BUILDDIR"/src/crinit-version.c 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-crinit

# run clang-tidy for unit tests
for d in "$BASEDIR"/test/utest/utest-*/; do
    SUBDIR="$d"
    clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$SUBDIR"/*.c 2>&1 \
        | tee "$BASEDIR"/result/clang-tidy/report-"$(basename "$SUBDIR")"
done

# run clang-tidy for test-specific headers
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/utest/*.h 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-testheaders

# run clang-tidy for mocks
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/utest/mocks/*.c "$BASEDIR"/test/utest/mocks/*.h 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-mocks
