# shellcheck shell=bash
#
# smoketest for starting and stopping the daemon
#

cleanup() {
    # Terminate crinit daemon
    crinit_daemon_stop
    # Delete temporary config.
    rm -f "${CONFDIR}"/demo.series
    rm -f "${CONFDIR}"/addseries/demoadd.series
    rm -f "${CONFDIR}"/crinit_recursive.crinit
}

trap "cleanup" EXIT

sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}#" < config/test/local.series > "${CONFDIR}"/demo.series
sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}/addseries#" < config/test/addseries/add.series > "${CONFDIR}"/addseries/demoadd.series

crinit_daemon_start "${CONFDIR}"/demo.series

echo "Crinit started with PID ${CRINIT_PID}."
