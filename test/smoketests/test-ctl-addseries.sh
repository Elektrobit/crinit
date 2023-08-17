# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit-ctl addseries command
#

setup() {
    crinit_config_setup
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add a new series
    if ! "${BINDIR}"/crinit-ctl addseries "${SMOKETESTS_CONFDIR}"/addseries/demoadd.series; then
        echo "crinit-ctl addseries failed unexpectedly"
        return 1
    fi

    out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".out
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    "$BINDIR"/crinit-ctl list | grep -o 'task_add_chain.' > "$out"
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

