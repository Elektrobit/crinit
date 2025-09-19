#!/bin/bash -e
# SPDX-License-Identifier: MIT
###############################################################################
print_info() {
    echo "
    Run the unit test suite or parts of it. Default is to run all unit
    tests for the Debug build.

    Usage: ${0} [build type] [--test-regex|-R <test name pattern>] [-h|--help]

    build type:       usually Debug or Release but can be any other build type
    --test-regex|-R:  execute all tests matching the pattern
    -h|--help:        print this help

    Examples:
    ${0} Release # run all unit test on Release build
    ${0} Release -R resize # run all unit test containing 'resize' in
    the name for the Release build.
    "
}
###############################################################################
set -e -u -o pipefail
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

TESTS_REGEX=""
PARAM=""
while [ $# -gt 0 ]; do
    case ${1} in
        --tests-regex | -R)
            TESTS_REGEX="--tests-regex ${2}"
            shift
            ;;
        -h | --help)
            print_info
            exit 0
            ;;
        -*)
            echo "error: unknown option: $1"
            print_info
            exit 1
            ;;
        *)
            PARAM="$PARAM ${1}"
            ;;
    esac
    shift
done
# shellcheck disable=SC2086 # Intended splitting of PARAM.
set -- $PARAM

BUILD_TYPE="${1:-Debug}"

BUILDDIR="${BASEDIR}/build/${BUILD_TYPE}"
CMAKE_BUILD_DIR="${BUILDDIR}/cmake"
RESULT_DIR="${BUILDDIR}/result/"

# check if ci/build.sh has been run before
if [ ! -d "$CMAKE_BUILD_DIR" ]; then
    echo Build environment \""${CMAKE_BUILD_DIR}"\" not set up. Please run ci/build.sh first!
    exit 1
fi

TEST_DIR="$CMAKE_BUILD_DIR"
mkdir -p "$RESULT_DIR/unit_test"
cd "$RESULT_DIR/unit_test"

# shellcheck disable=SC2086 # Intended splitting of TESTS_REGEX.
ctest --output-on-failure --force-new-ctest-process --verbose \
    --output-junit "$RESULT_DIR/unit_test/junit.xml" \
    --no-compress-output \
    --output-log "$RESULT_DIR/unit_test/Test.log" \
    --test-dir "$TEST_DIR" \
    ${TESTS_REGEX}
RETURNCODE=$?

cp -r "${CMAKE_BUILD_DIR}/Testing/Temporary" "${RESULT_DIR}/unit_test/"

TEST_LOG_FILE="${TEST_DIR}/Testing/Temporary/LastTest.log"
SKIPPED_TESTS=$(sed -n -e '/^# skip/p' "${TEST_LOG_FILE}" | wc -l)
if [ "${SKIPPED_TESTS}" -gt 0 ]; then
    echo "Skipped tests (${SKIPPED_TESTS}):"
    grep "# skip " "${TEST_LOG_FILE}"
fi

exit $RETURNCODE
