#!/bin/bash
# SPDX-License-Identifier: MIT
#
# Script linter tool, currently for Python, shellcheck planned.
#
# Usage: ./ci/lint-scripts.sh
#
# The --check parameter causes the script to only do a dry-run and error out
# if file changes would be necessary. This funcitonality is mainly meant for
# the CI.
#
# Set FLAKE8_CMD env variable if your code linting tool is called differently
# or not in ${PATH}.
#

set -euo pipefail

FLAKE8_CMD=${FLAKE8_CMD:-flake8}
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# Use our repository Flake8 configuration (e.g. for custom line width)
FLAKE8_CONF_ARGS="--config ${BASEDIR}/.flake8"

FAIL=0

pop_back() {
    popd >/dev/null
}

pushd "${BASEDIR}" >/dev/null
trap pop_back EXIT

# shellcheck disable=SC2086 # Intended splitting of FLAKE8_CONF_ARGS.
if ! ${FLAKE8_CMD} ${FLAKE8_CONF_ARGS}; then
    FAIL=1
    echo Flake8 found the above issues in Python code, please fix. 1>&2
fi

exit ${FAIL}
