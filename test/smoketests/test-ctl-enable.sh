# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for crinit-ctl enable/disable commands
#

TMPDIR="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".tmp

setup() {
    crinit_config_setup
    rm -f /tmp/crinit-one_second_respawn.log
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! "${BINDIR}"/crinit-ctl enable one_second_respawn; then
        echo "crinit-ctl enable failed"
        return 1
    fi

    sleep 5

    if ! "${BINDIR}"/crinit-ctl disable one_second_respawn; then
        echo "crinit-ctl disable failed"
        return 1
    fi

    sleep 1

    count=$(grep -c 'A second is over' < /tmp/crinit-one_second_respawn.log)
    if [ "$count" -lt 1 ]; then
        echo "wrong execution count: $count"
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

