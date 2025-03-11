# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit stop command execution
#

setup() {
    crinit_config_setup
    if [ -e /tmp/test-stop_command.out ]; then
        rm /tmp/test-stop_command.out
    fi
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! "${BINDIR}"/crinit-ctl stop stop_command; then
        echo "crinit-ctl stop failed"
        return 1
    fi

    sleep 1

    out="/tmp/test-stop_command.out"
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
