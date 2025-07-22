#!/bin/bash

CMD_PATH="$(realpath "$(dirname "$0")")"
BASE_DIR="$(realpath "$CMD_PATH/..")"
export BUILD_TYPE=amd64
export BUILD_DIR="${BASE_DIR}/build/${BUILD_TYPE}"
export RESULT_DIR="${BUILD_DIR}/results"

mkdir -p "${RESULT_DIR}"

"${BASE_DIR}"/test/coverage/run_asmcov.sh
STATUS=$?

find "${RESULT_DIR}/coverage_results" -name "*.trace" -delete

exit "${STATUS}"
