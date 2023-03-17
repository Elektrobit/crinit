# shellcheck shell=bash
#
# smoketest for testing the parser of COMMANDs including quoting and escaping.
#

task="${SMOKETESTS_CONFDIR}"/command_parsing_test.crinit
out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-out.txt
sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.txt

# COMMAND[] = "/bin/sh" -c    "echo \\" ''' \\t \\xe2\\x99\\xa5  \\" | tee ${out}"


setup() {
    crinit_config_setup
    rm -f "${out}"
    rm -f "${task}"
    cat << EOF > "${task}"
# Config to test features of the parser in COMMAND values.

NAME = command_parsing_test

COMMAND[] = /bin/sh "-c"   "echo \\" '''   \\xe2\\x99\\xa5  \\"\\x68ello w\\ spc | tee ${out}"

DEPENDS = ""

RESPAWN = NO

# features below not yet implemented
EXEC = NO
QM_JAIL = NO
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add a new task
    if ! "${BINDIR}"/crinit-ctl addtask "${task}"; then
        echo "crinit-ctl addtask failed unexpectedly"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q command_parsing_test; then
        echo "crinit-ctl addtask successful, but task not in list"
        return 1
    fi

    sleep 1
    compare_output "${sample}" "${out}"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

