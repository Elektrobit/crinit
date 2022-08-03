# shellcheck shell=bash
#
# smoketest for crinit-ctl kill command
#

setup() {
    crinit_config_setup
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}#" < "${SMOKETESTS_CONFDIR}"/local.series > "${SMOKETESTS_CONFDIR}"/demo.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd.series
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

    out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".out
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    "$BINDIR"/crinit-ctl status sleep_one_day > "$out"
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

