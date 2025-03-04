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

    # add a new task, with unresolvable dependencies
    if ! "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/ignore_deps.crinit; then
        echo "crinit-ctl addtask failed unexpectedly"
        return 1
    fi

    sleep 1

    # Check if it was loaded and if it is blocked by its unresolvable dependencies
    if ! "$BINDIR"/crinit-ctl list | grep ignore_deps | grep -q loaded; then
        echo "crinit-ctl addtask successful, but task not in list or task was unexpectedly run."
        return 1
    fi

    # overwrite it with no dependencies
    if ! "${BINDIR}"/crinit-ctl addtask -fi "${SMOKETESTS_CONFDIR}"/ignore_deps.crinit; then
        echo "crinit-ctl addtask -fi failed unexpectedly"
        return 1
    fi

    sleep 1

    # check if it has now been run
    if ! "$BINDIR"/crinit-ctl list | grep ignore_deps | grep -q "done"; then
        echo "crinit-ctl addtask -fi successful, but task not in list or task was not run."
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
