# shellcheck shell=bash
#
# smoketest for starting and stopping the daemon
#

setup() {
    sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}#" < config/test/local.series > "${CONFDIR}"/demo.series
    sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}/addseries#" < config/test/addseries/add.series > "${CONFDIR}"/addseries/demoadd.series
}

run() {
    crinit_daemon_start "${CONFDIR}"/demo.series
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
    # Delete temporary config.
    rm -f "${CONFDIR}"/demo.series
    rm -f "${CONFDIR}"/addseries/demoadd.series
}
