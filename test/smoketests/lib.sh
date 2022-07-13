# shellcheck shell=bash
#
# smoketests library functions
#
# This file should be sourced from smoketests to provide
# common functionality for tests.
#

CRINIT_PID=

crinit_daemon_start() {
    sudo valgrind --leak-check=full "${BINDIR}"/crinit "$@" &
    SUDO_PID="$!"
    sleep 1
    CRINIT_PID=$(ps -o pid= --ppid ${SUDO_PID})
}

crinit_daemon_stop() {
    if [ -n "$CRINIT_PID" ]; then
        sudo kill "$CRINIT_PID"
    fi
}
