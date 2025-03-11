# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for respawn retries
#

TMPDIR="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".tmp

setup() {
    crinit_config_setup
    rm -f /tmp/crinit-fail_loop.log
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! "${BINDIR}"/crinit-ctl enable fail_loop; then
        echo "crinit-ctl enable failed"
        return 1
    fi

    sleep 5

    if ! "${BINDIR}"/crinit-ctl disable fail_loop; then
        echo "crinit-ctl disable failed"
        return 1
    fi

    sleep 1

    out="/tmp/crinit-fail_loop.log"
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
