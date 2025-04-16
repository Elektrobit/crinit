#!/bin/bash
# SPDX-License-Identifier: MIT
#
print_info() {
    echo "\
Script linter tool for Python and sh/bash.

Usage: $0

The script will run shellcheck and flake8 to lint shell and python code in the
project. It will return 1 if there are any linter complaints, 0 otherwise.

Set FLAKE8_CMD and/or SHELLCHK_CMD env variables if your code linting tools
(flake8 and shellcheck) are called differently  or not available in \${PATH}."
}

set -euo pipefail

# Check if we've been given any arguments. If so, print usage info and exit.
if [ $# -gt 0 ]; then
    print_info
    exit 1
fi

FLAKE8_CMD=${FLAKE8_CMD:-flake8}
SHELLCHK_CMD=${SHELLCHK_CMD:-shellcheck}
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# The dirs we want to search for sh files. Not needed for python as flake8
# will crawl all subdirs by default.
SH_CODE_DIRS="${BASEDIR}/ci ${BASEDIR}/scripts ${BASEDIR}/test"
# Let shellcheck follow source statements.
SHELLCHK_ARGS="-x"

# Use our repository Flake8 configuration (e.g. for custom line width)
FLAKE8_CONF_ARGS="--config ${BASEDIR}/.flake8"

FAIL=0

pop_back() {
    # shellcheck disable=SC2317 # False positive "unreachable statement" due to trap.
    popd >/dev/null
}

pushd "${BASEDIR}" >/dev/null
trap pop_back EXIT

# shellcheck disable=SC2086 # Intended splitting of FLAKE8_CONF_ARGS.
if ! ${FLAKE8_CMD} ${FLAKE8_CONF_ARGS}; then
    FAIL=1
    echo Flake8 found the above issues in Python code, please fix. 1>&2
fi

# shellcheck disable=SC2086 # Intended splitting of SH_CODE_DIRS, SHELLCHK_ARGS.
if ! find ${SH_CODE_DIRS} -iname '*.sh' -print0 | xargs -0 "${SHELLCHK_CMD}" ${SHELLCHK_ARGS}; then
    FAIL=1
    echo Shellcheck found the above issues in sh/bash code, please fix. 1>&2
fi

exit ${FAIL}
