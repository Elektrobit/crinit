#!/bin/bash -e
#
# script to prepare and run clang-tidy on the project
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}
BUILDDIR=build/aarch64

if [ ! -f "${BUILDDIR}"/compile_commands.json ]; then
    echo "Build environment not set up. Please run ci/build.sh first!" >&2
    exit 1
fi

rm -rf "$BASEDIR"/result/clang-tidy
mkdir "$BASEDIR"/result/clang-tidy
cd "$BASEDIR"

CLANG_TIDY_FLAGS=( -p "${BUILDDIR}" )

# run clang-tidy for crinit
clang-tidy "${CLANG_TIDY_FLAGS[@]}" -dump-config "$BASEDIR"/inc/*.h "$BASEDIR"/src/*.c "$BASEDIR"/"$BUILDDIR"/src/version.c \
    > "$BASEDIR"/result/clang-tidy/config
# catch errors even though we use a pipe to tee
set -o pipefail
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/inc/*.h "$BASEDIR"/src/*.c "$BASEDIR"/"$BUILDDIR"/src/version.c 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-crinit

# run clang-tidy for unit tests
for d in "$BASEDIR"/test/utest-*/ ; do
    SUBDIR="$d"
    clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$SUBDIR"/*.c 2>&1 \
        | tee "$BASEDIR"/result/clang-tidy/report-"$(basename "$SUBDIR")"
done

# run clang-tidy for test-specific headers
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/*.h 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-testheaders

# run clang-tidy for mocks
clang-tidy "${CLANG_TIDY_FLAGS[@]}" "$BASEDIR"/test/mocks/*.c "$BASEDIR"/test/mocks/*.h 2>&1 \
    | tee "$BASEDIR"/result/clang-tidy/report-mocks
