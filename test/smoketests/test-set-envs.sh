# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for setting of environment variables
#

task="${SMOKETESTS_CONFDIR}"/smoketest_env.crinit
out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-out.env
sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.env
varnames="GLOB_VAR OVR_VAR LOC_VAR ESCSEQ_VAR ESC_VAR CRINIT_TASK_NAME GLOB_REF EXP_VAR HEX_ESC_VAR EMPTY_VAR"

setup() {
    crinit_config_setup
    rm -f ${out}
    rm -f ${task}
    cat <<EOF >${task}
# Config to show off environment settings.

NAME = smoketest_env

COMMAND[0] = /bin/sh -c "export -p | tee ${out}"

DEPENDS = ""

RESPAWN = NO

ENV_SET = LOC_VAR "Local EnvVar"
ENV_SET = OVR_VAR "Hello, local World!"
          GLOB_REF "Substituted global var: <\${GLOB_VAR}>"
ENV_SET = ESC_VAR "Escaped variable: \\\${ESC_VAR}"

FILTER_DEFINE = SSH_CONN ".event.source.appName 'ssh' STRCMP .event.payload r'*Accepted*' REGEX AND"
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add a new task
    if ! "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/smoketest_env.crinit; then
        echo "crinit-ctl addtask failed unexpectedly"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q smoketest_env; then
        echo "crinit-ctl addtask successful, but task not in list"
        return 1
    fi

    sleep 1

    . ${sample}
    . ${out}

    for x in ${varnames}; do
        RESULT=$(eval echo "\${${x}}")
        EXPECTED=$(eval echo "\${EXPECTED_${x}}")

        if [ "${RESULT}" != "${EXPECTED}" ]; then
            echo "Mismatch between actual and expected value for environment variable \'${x}\'."
            echo "Expected result: ${EXPECTED}"
            echo "Actual result: ${RESULT}"
            return 1
        fi
    done
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}
