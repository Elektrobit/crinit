# shellcheck shell=bash
#
# smoketests library functions
#
# This file should be sourced from smoketests to provide
# common functionality for tests.
#

CRINIT_PID=
CRINIT_SOCK=/run/crinit/crinit.sock

crinit_daemon_start() {
    local cmd=( "${BINDIR}"/crinit )

    if [ "$USE_VALGRIND" -eq 1 ]; then
        cmd=( valgrind --leak-check=full --log-file="$RESULTDIR/${SMOKETESTS_NAME}-valgrind.log" "${cmd[@]}" )
    fi

    # shellcheck disable=SC2024
    sudo "${cmd[@]}" "$@" > "$RESULTDIR/${SMOKETESTS_NAME}-crinit.log" 2>&1 &
    CRINIT_PID="$!"

    # give crinit some time to initialize in background
    for (( i = 0; i < 10; i++ )); do
        if kill -0 "$CRINIT_PID"; then
            if [ -e "$CRINIT_SOCK" ]; then
                echo "Crinit started with PID ${CRINIT_PID}."
                return 0
            fi
        fi
        sleep 1
    done

    return 1
}

crinit_daemon_stop() {
    if [ -n "$CRINIT_PID" ]; then
        sudo kill "$CRINIT_PID"
    fi
}
