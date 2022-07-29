# shellcheck shell=sh
#
# smoketests library functions
#
# This file should be sourced from smoketests to provide
# common functionality for tests.
#

if [ -z "$BINDIR" ]; then
    BINDIR=/usr/bin
fi
if [ -z "$CONFDIR" ]; then
    CONFDIR=config/test
fi
if [ -z "$SMOKETESTS_RESULTDIR" ]; then
    SMOKETESTS_RESULTDIR=/tmp/smoketests
    mkdir -p "$SMOKETESTS_RESULTDIR"
fi

CRINIT_PID=
CRINIT_SOCK=/run/crinit/crinit.sock

crinit_daemon_start() {
    set -- "${BINDIR}"/crinit "$@"

    if [ -n "$SMOKETESTS_VALGRIND" ] && [ "$SMOKETESTS_VALGRIND" -eq 1 ]; then
        set -- valgrind --leak-check=full --log-file="${SMOKETESTS_RESULTDIR}/${SMOKETESTS_NAME}-valgrind.log" "$@"
    fi

    # remove stale socket if it exists
    sudo rm -f "$CRINIT_SOCK"

    # shellcheck disable=SC2024
    sudo -E "$@" > "${SMOKETESTS_RESULTDIR}/${SMOKETESTS_NAME}-crinit.log" 2>&1 &
    CRINIT_PID="$!"

    # give crinit some time to initialize in background
    i=10
    while [ $i -gt 0 ]; do
        if kill -0 "$CRINIT_PID"; then
            if [ -e "$CRINIT_SOCK" ]; then
                echo "Crinit started with PID ${CRINIT_PID}."
                return 0
            fi
        fi
        sleep 1
        : $(( i -= 1 ))
    done

    return 1
}

crinit_daemon_stop() {
    if [ -n "$CRINIT_PID" ]; then
        if ! sudo kill "$CRINIT_PID"; then
            echo "Crinit with PID ${CRINIT_PID} already exited?"
            return 1
        fi
    fi

    return 0
}
