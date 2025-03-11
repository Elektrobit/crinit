# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for starting and stopping the daemon
#

setup() {
    crinit_config_setup
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}#" <"${SMOKETESTS_CONFDIR}"/local.series >"${SMOKETESTS_CONFDIR}"/demo.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" <"${SMOKETESTS_CONFDIR}"/addseries/add.series >"${SMOKETESTS_CONFDIR}"/addseries/demoadd.series
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
