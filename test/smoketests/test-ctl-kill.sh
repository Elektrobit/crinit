# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit-ctl kill command
#

setup() {
    crinit_config_setup
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/sleep_one_day.crinit; then
        echo "crinit-ctl addtask failed"
        return 1
    fi

    sleep 3

    if ! "$BINDIR"/crinit-ctl kill sleep_one_day; then
        echo "crinit-ctl kill failed"
        return 1
    fi

    sleep 1

    if ! crinit_task_check_status "sleep_one_day" "failed"; then
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
