# shellcheck shell=bash
#
# smoketest for crinit-ctl status command
#

setup() {
    crinit_config_setup
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}#" < "${SMOKETESTS_CONFDIR}"/local.series > "${SMOKETESTS_CONFDIR}"/demo.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd.series
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".out
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    "$BINDIR"/crinit-ctl status after_sleep > "$out"
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

