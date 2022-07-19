#!/bin/sh -e

set -x

CMDPATH=$(cd "$(dirname "$0")" && pwd)

# shellcheck source=test/smoketests/lib.sh
. "$CMDPATH"/lib.sh

NUM=0
NUMOK=0

for t in "$CMDPATH"/test-*.sh; do
    : $(( NUM += 1 ))

    SMOKETESTS_NAME="${t##*/test-}"
    SMOKETESTS_NAME="${SMOKETESTS_NAME%.sh}"
    echo "--> Running test $SMOKETESTS_NAME" >&2

    . "$t"

    exitcode=0
    setup || exitcode=$?
    if [ "$exitcode" -ne 0 ]; then
        echo "$SMOKETESTS_NAME: setup failed" >&2
    else
        exitcode=0
        run || exitcode=$?
        if [ "$exitcode" -ne 0 ]; then
            echo "$SMOKETESTS_NAME: run failed" >&2
        else
            echo "$SMOKETESTS_NAME: run success" >&2
        fi
    fi
    exitcode=0
    teardown || exitcode=$?
    if [ "$exitcode" -ne 0 ]; then
        echo "$SMOKETESTS_NAME: teardown failed" >&2
        break
    fi

    : $(( NUMOK += 1 ))
done

NUMFAIL=$(( NUM - NUMOK ))

echo ""
echo "--> $NUM tests: $NUMOK success, $NUMFAIL failed"
