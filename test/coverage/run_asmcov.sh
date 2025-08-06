#!/bin/bash
set -eu

CMDPATH=${0%/*}
BASE_DIR=$(realpath "${CMDPATH}/../..")
BUILD_DIR="${BUILD_DIR:-${BASE_DIR}/build/Debug}"
ASMCOV_DIR=${ASMCOV_DIR-"./"}

TEXT_RED="\033[31m"
TEXT_GREEN="\033[32m"
TEXT_YELLOW="\033[33m"
TEXT_RESET="\033[m"

export COVERAGE_DIR=${COVERAGE_DIR-"$(dirname "$0")"}
export COVERAGE_RESULT_DIR=${COVERAGE_RESULT_DIR-"${BUILD_DIR}/result/coverage/asmcov"}

function is_build_with_asan {
    test x != x"$(ldd "$1" | grep libasan)"
}

function run_test {
    local METHOD="$1"
    local BINARY_UNDER_TEST="$2"
    local TEST_NAME
    local RESULT_DIR

    TEST_NAME=$(basename "${BINARY_UNDER_TEST}")
    RESULT_DIR="${COVERAGE_RESULT_DIR}/${TEST_NAME}"

    mkdir -p "${RESULT_DIR}"

    local PERF_DATA_FILE="${RESULT_DIR}/${TEST_NAME}.data"
    local PERF_TRACE_FILE="${RESULT_DIR}/${TEST_NAME}.trace"

    if is_build_with_asan "${BINARY_UNDER_TEST}"; then
        echo "WARNING: code is instrumented by address sanitizer, asmcov results
    will probably reveal more branches then expected from the plain source
    code!" | tee -a "${RESULT_DIR}/asmcov.log"
    fi
    echo "##### Coverage for ${METHOD}"
    LD_BIND_NOW=1 perf record -o "${PERF_DATA_FILE}" --event intel_pt//u "${BINARY_UNDER_TEST}"
    perf script -i "${PERF_DATA_FILE}" --itrace=i0 --fields dso,sym,ip,insn,insnlen --show-mmap-events >"${PERF_TRACE_FILE}"
    cd "${RESULT_DIR}"
    set +e -x
    asmcov "${BINARY_UNDER_TEST}" "${METHOD}" "${PERF_TRACE_FILE}" 2>&1 | tee -a "${RESULT_DIR}/asmcov.log"
    set -e +x
    cd -
}

function print_function_coverage {
    FUNCTION_SYMBOLS_LOG="${COVERAGE_RESULT_DIR}/symbols.log"
    FUNCTION_COVERAGE_LOG="${COVERAGE_RESULT_DIR}/function_coverage.log"
    find "$BUILD_DIR/cmake/src" -type f -executable -exec objdump -t {} \; >"${FUNCTION_SYMBOLS_LOG}"

    PUBLIC_FUNCTIONS=$(grep "g[ ]*F .text" "${FUNCTION_SYMBOLS_LOG}" | grep -oE "[^ ]+$")
    PUBLIC_FUNCTION_COUNT=$(echo "${PUBLIC_FUNCTIONS}" | sort -u | wc -w)
    echo "Public functions: ${PUBLIC_FUNCTIONS}" | sort -u >>"${FUNCTION_COVERAGE_LOG}"

    LOCAL_FUNCTIONS=$(grep "l[ ]*F .text" "${FUNCTION_SYMBOLS_LOG}" | grep -oE "[^ ]+$")
    LOCAL_FUNCTION_COUNT=$(echo "${LOCAL_FUNCTIONS}" | sort -u | wc -w)
    echo "Local functions: ${LOCAL_FUNCTIONS}" | sort -u >>"${FUNCTION_COVERAGE_LOG}"

    TESTED_PUBLIC_FUNCTIONS=$(find "${COVERAGE_RESULT_DIR}" -mindepth 1 -type d -printf "%f ")
    TESTED_PUBLIC_FUNCTION_COUNT=$(echo "${TESTED_PUBLIC_FUNCTIONS}" | wc -w)

    echo "coverage for tests : ${TESTED_PUBLIC_FUNCTIONS}"
    local COVERAGE=0
    for TEST_NAME in ${TESTED_PUBLIC_FUNCTIONS}; do
        echo "--> ${TEST_NAME}"
        local RESULT_DIR=${COVERAGE_RESULT_DIR}/${TEST_NAME}
        if [ -f "${RESULT_DIR}/asmcov.log" ]; then
            local VALUE
            VALUE=$(grep "Branch coverage:" "${RESULT_DIR}/asmcov.log" | tr -dc 0-9)
            if [ "x$VALUE" != "x" ]; then
                COVERAGE=$((COVERAGE + VALUE))
            fi
        else
            echo "Skip ${RESULT_DIR}, look not like a test result dir"
        fi
    done

    if [ "${TESTED_PUBLIC_FUNCTION_COUNT}" -gt 0 ]; then
        TESTED_COVERAGE=$((COVERAGE / TESTED_PUBLIC_FUNCTION_COUNT))
    else
        TESTED_COVERAGE=0
    fi

    if [ "${PUBLIC_FUNCTION_COUNT}" -gt 0 ]; then
        TOTAL_COVERAGE=$((COVERAGE / PUBLIC_FUNCTION_COUNT))
    else
        TOTAL_COVERAGE=0
    fi

    echo "tested coverage: ${TESTED_COVERAGE}%"
    echo "total coverage: ${TOTAL_COVERAGE}%"
    echo "crinit.coverage" "build/Release/result" "public_functions=${PUBLIC_FUNCTION_COUNT},tested_public_functions=${TESTED_PUBLIC_FUNCTION_COUNT},local_functions=${LOCAL_FUNCTION_COUNT},average_coverage=${TOTAL_COVERAGE},average_tested_coverage=${TESTED_COVERAGE}"

    rm "${FUNCTION_SYMBOLS_LOG}"
}

function print_short_report {
    local COVERAGE
    local TEST_LOG
    local TEST_DIR
    local TEST_NAME

    echo "#################################################"
    echo "## Short report"
    echo "#################################################"

    for TEST_LOG in "${COVERAGE_RESULT_DIR}/"*"/asmcov.log"; do
        TEST_DIR="$(dirname "${TEST_LOG}")"
        TEST_NAME="$(basename "${TEST_DIR}")"
        COVERAGE=$(grep "Branch coverage:" "${TEST_LOG}")

        case "${COVERAGE}" in
            "Branch coverage:	100%")
                echo -e "${TEXT_GREEN} ${COVERAGE##*:} ${TEXT_RESET} ${TEST_NAME}"
                ;;
            "Branch coverage:	0%")
                echo -e "${TEXT_RED} ${COVERAGE##*:} ${TEXT_RESET} ${TEST_NAME}"
                ;;
            "Branch coverage:"*)
                echo -e "${TEXT_YELLOW} ${COVERAGE##*:} ${TEXT_RESET} ${TEST_NAME}"
                ;;
            *)
                echo -e "${TEXT_RED} \t0% ${TEXT_RESET} ${TEST_NAME}"
                ;;
        esac
    done
}

create_coverage_reports() {
    while read -r line; do
        FUNCTION_UNDER_TEST=${line%% *}
        TEST_BINARY=${line#* }

        echo "Function under test: ${FUNCTION_UNDER_TEST}"
        echo "Test binary: ${TEST_BINARY}"

        # check if symbol of function under test was renamed due to mocking
        if objdump "${TEST_BINARY}" -t | grep "__real_${FUNCTION_UNDER_TEST}"; then
            run_test "__real_${FUNCTION_UNDER_TEST}" "${TEST_BINARY}"
        else
            run_test "${FUNCTION_UNDER_TEST}" "${TEST_BINARY}"
        fi
    done <"$1/functions_under_test.txt"
}

if [ ! -d "${BUILD_DIR}/cmake/test" ]; then
    echo "No test directory found in ${BUILD_DIR}. Run ${BASE_DIR}/ci/build.sh or set BUILD_DIR."
    exit 1
fi

rm -rf "${COVERAGE_RESULT_DIR}"
mkdir -p "${COVERAGE_RESULT_DIR}"

create_coverage_reports "${BUILD_DIR}/cmake/test/"

print_short_report | tee -a "${COVERAGE_RESULT_DIR}/asmcov_short_report.log"
print_function_coverage | tee -a "${COVERAGE_RESULT_DIR}/asmcov_short_report.log"
