# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit-ctl status command
#

setup() {
    crinit_config_setup
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! crinit_task_check_status "after_sleep" "done"; then
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

