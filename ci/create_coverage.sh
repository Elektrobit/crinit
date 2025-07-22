#!/bin/bash
# SPDX-License-Identifier: MIT
###############################################################################
print_info() {
    echo "
    Create coverage report for crinit. Either using asmocv or gcov.

    Usage: ${0} [coverage type] [-h|--help]

    coverage type:    Defien the coverage tool to use either 'asmcov' or
                      'gcov'. Default is 'asmcov'
    -h|--help:        print this help

    Examples:
    ${0} gcov   # create gcov report
    ${0} asmcov # create asmcov report
    "
}
###############################################################################
set -e -u -o pipefail

CMD_PATH="$(realpath "$(dirname "$0")")"
BASE_DIR="$(realpath "$CMD_PATH/..")"
export BUILD_TYPE=amd64
export BUILD_DIR="${BASE_DIR}/build/${BUILD_TYPE}"
export RESULT_DIR="${BUILD_DIR}/results"

PARAM=""
while [ $# -gt 0 ]; do
    case ${1} in
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
# shellcheck disable=SC2086 # Intended splitting of PARAM
set -- $PARAM

mkdir -p "${RESULT_DIR}"
COVERAGE_TYPE=${1:-"asmcov"}

case "${COVERAGE_TYPE}" in
    gcov)
        if [ ! -d "${BUILD_DIR}-coverage/" ]; then
            echo "Fail, assumed coverage build in ${BUILD_DIR}-coverage/"
            exit 1
        fi
        mkdir -p "${BUILD_DIR}-coverage/results/coverage"
        gcovr --output "${BUILD_DIR}-coverage/results/coverage/gcov_report.html"
        STATUS=$?
        ;;
    *)
        "${BASE_DIR}/test/coverage/run_asmcov.sh"
        STATUS=$?
        find "${RESULT_DIR}/coverage/asmcov" -name "*.trace" -delete
        ;;
esac

exit "${STATUS}"
