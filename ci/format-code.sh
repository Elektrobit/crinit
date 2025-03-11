#!/bin/bash
# SPDX-License-Identifier: MIT
#
# Code formatter and formatting checker script. Uses shfmt (sh/bash) and
# clang-format (C).
#
# Usage: ./ci/format-code.sh [--check]
#
# The --check parameter causes the script to only do a dry-run and error out
# if file changes would be necessary. This funcitonality is mainly meant for
# the CI.
#
# Set CLANG_FORMAT_CMD and/or SHFMT_CMD env variable if your clang-format
# and/or shfmt tools are somewhere else than ${PATH}.
#

set -euo pipefail

CLANG_FORMAT_CMD=${CLANG_FORMAT_CMD:-clang-format}
SHFMT_CMD=${SHFMT_CMD:-shfmt}
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

print_usage() {
    echo "Usage: $0 [--check]" 1>&2
    echo "Formats shell and C code in this repo. If \'--check\' is specified," \
        "it will only do a dry-run and report an error code if files would be" \
        "changed. This functionality is mainly meant for the CI." 1>&2
}

# Google shell style for shfmt with 4 space indentation instead of 2.
SHFMT_STYLE_ARGS="-i 4 -bn -ci"
# For clang-format, we just use our existing style file.
CLANG_FORMAT_STYLE_ARGS="-style=file:${BASEDIR}/.clang-format"

# By default edit the files:
SHFMT_ACTION_ARGS="-w"
CLANG_FORMAT_ACTION_ARGS="-i"

# The directories we want to search in for both tools.
C_CODE_DIRS="${BASEDIR}/inc ${BASEDIR}/src"
SH_CODE_DIRS="${BASEDIR}/ci ${BASEDIR}/scripts ${BASEDIR}/test"

if [[ $# == 1 ]]; then
    if [ "$1" = "--check" ]; then
        # do dry-runs instead
        SHFMT_ACTION_ARGS="-l"                # Will list files that would be changed.
        CLANG_FORMAT_ACTION_ARGS="-n -Werror" # Will do a dry-run and report error if files need to be changed.
    else
        print_usage
        exit 1
    fi
elif [[ $# != 0 ]]; then
    print_usage
    exit 1
fi

FAIL=0

# shellcheck disable=SC2086 # Intended splitting of C_CODE_DIRS, CLANG_FORMAT_STYLE_ARGS, CLANG_FORMAT_ACTION_ARGS
if ! find ${C_CODE_DIRS} -iname '*.h' -print0 -or -iname '*.c' -print0 \
    | xargs -0 "${CLANG_FORMAT_CMD}" ${CLANG_FORMAT_STYLE_ARGS} ${CLANG_FORMAT_ACTION_ARGS}; then
    FAIL=1
    echo Clang-format found the above formatting issues, please fix. 1>&2
fi

# shellcheck disable=SC2086 # Intended splitting of SH_CODE_DIRS, SHFMT_STYLE_ARGS, SHFMT_ACTION_ARGS
if [[ "$(find ${SH_CODE_DIRS} -iname '*.sh' -print0 \
    | xargs -0 "${SHFMT_CMD}" ${SHFMT_STYLE_ARGS} ${SHFMT_ACTION_ARGS})" ]]; then
    FAIL=1
    echo Shfmt found formatting issues in some shell script files, please fix. 1>&2
fi

exit ${FAIL}
