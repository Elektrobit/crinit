#!/bin/bash
# SPDX-License-Identifier: MIT
#
# Commit message lint script for merge request branches.
#
# Usage: ./ci/lint-commits.sh <TARGET_BRANCH>
#
# Set COMMITLINT_CMD env variable if your commitlint binary/script is not in
# your path or if you want to run using npx.
#

set -euo pipefail

COMMITLINT_CMD=${COMMITLINT_CMD:-commitlint}
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

if [[ $# -ne 1 ]]; then
    echo "Usage: ./ci/lint-commits.sh <TARGET_BRANCH>" 1>&2
    echo Set COMMITLINT_CMD env variable if your commitlint binary/script is \
        not in your path or if you want to run using npx. 1>&2
    exit 1
fi

cleanup() {
    popd >/dev/null
}

pushd "${BASEDIR}" >/dev/null
trap cleanup EXIT

FAIL=0

if ! ${COMMITLINT_CMD} -f "$1"; then
    FAIL=1
    echo Commitlint found errors, please fix. 1>&2
fi

if git log --format="%s" "$1.." | grep -q "^fixup!"; then
    FAIL=1
    echo There seem to be fixup commits still present in this branch, \
        please "(auto-)squash."
fi

exit ${FAIL}
