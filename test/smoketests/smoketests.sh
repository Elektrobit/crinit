#!/bin/sh -e
# SPDX-License-Identifier: MIT

if [ -n "$SMOKETESTS_DEBUG" ] && [ "$SMOKETESTS_DEBUG" -eq 1 ]; then
    set -x
fi

CMDPATH=$(cd "$(dirname "$0")" && pwd)

# shellcheck source=test/smoketests/lib.sh
. "$CMDPATH"/lib.sh

# clear previous results
rm -rf "$SMOKETESTS_RESULTDIR"
mkdir -p "$SMOKETESTS_RESULTDIR"

NUM=0
NUMOK=0

for t in "$CMDPATH"/test-*.sh; do
    : $((NUM += 1))

    SMOKETESTS_NAME="${t##*/test-}"
    SMOKETESTS_NAME="${SMOKETESTS_NAME%.sh}"
    echo "--> Running test $SMOKETESTS_NAME" >&2

    # shellcheck disable=SC1090 # The test-*.sh scripts are linted separately, so no need to follow.
    . "$t"

    export ASAN_OPTIONS="${ASAN_OPTIONS}:log_path=${SMOKETESTS_RESULTDIR}/${SMOKETESTS_NAME}-asan"

    success=1

    exitcode=0
    setup || exitcode=$?
    if [ "$exitcode" -ne 0 ]; then
        echo "$SMOKETESTS_NAME: setup failed" >&2
        success=0
    else
        exitcode=0
        run || exitcode=$?
        if [ "$exitcode" -ne 0 ]; then
            echo "$SMOKETESTS_NAME: run failed" >&2
            success=0
        else
            echo "$SMOKETESTS_NAME: run success" >&2
        fi
    fi
    exitcode=0
    teardown || exitcode=$?
    if [ "$exitcode" -ne 0 ]; then
        echo "$SMOKETESTS_NAME: teardown failed" >&2
        success=0
    fi

    : $((NUMOK += success))

    if [ "$success" -ne 1 ] && [ "$SMOKETESTS_FAILSTOP" -eq 1 ]; then
        break
    fi
done

NUMFAIL=$((NUM - NUMOK))

echo ""
echo "--> $NUM tests: $NUMOK success, $NUMFAIL failed"

if [ -z "$NUMFAIL" ] || [ "$NUMFAIL" -ne 0 ]; then
    exit 1
fi
exit 0
