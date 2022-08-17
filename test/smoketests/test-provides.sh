# shellcheck shell=bash
#
# smoketest for crinit feature-based depnedencies commands
#

setup() {
    crinit_config_setup
    rm -f /tmp/crinit-feature_dependent.log
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    if ! "${BINDIR}"/crinit-ctl enable feature_provider; then
        echo "crinit-ctl enable of feature provider task failed"
        return 1
    fi
 
    sleep 1

    out="/tmp/crinit-feature_dependent.log"
    sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}".out
    compare_output "$sample" "$out"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

