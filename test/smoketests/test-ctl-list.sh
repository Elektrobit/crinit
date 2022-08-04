# shellcheck shell=bash
#
# smoketest for crinit-ctl list command
#

setup() {
    crinit_config_setup
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}".out
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    "$BINDIR"/crinit-ctl list > "$out"
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

