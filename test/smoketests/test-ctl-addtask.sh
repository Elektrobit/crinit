# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit-ctl addtask command
#

setup() {
    crinit_config_setup
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add a new task
    if ! "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/sleep_one_day.crinit; then
        echo "crinit-ctl addtask failed unexpectedly"
        return 1
    fi

    # try to add it again, which should fail
    if "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/sleep_one_day.crinit; then
        echo "crinit-ctl addtask should have failed"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q sleep_one_day; then
        echo "crinit-ctl addtask successful, but task not in list"
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

