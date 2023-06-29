# shellcheck shell=bash
#
# smoketest for include support
#

task="${SMOKETESTS_CONFDIR}"/smoketest_incl.crinit
out="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-out.env
sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.env
varnames="OVR_VAR LOC_VAR INCLUDED_ENV FAKE_ENV"

setup() {
    crinit_config_setup
    rm -f ${out}
    rm -f ${task}
    cat << EOF > ${task}
# Config to show off environment settings.

NAME = smoketest_incl

# This IO redirection has to come before the one in the second include file to work
IO_REDIRECT = STDOUT "${out}" APPEND

INCLUDE = incl_test_first
          incl_test_second DEPENDS,IO_REDIRECT

COMMAND = /bin/sh -c "export -p"
          /bin/sh -c "echo export FAKE_ENV='stderr' 1>&2"

DEPENDS = ""

RESPAWN = NO

ENV_SET = LOC_VAR "Local EnvVar"
          OVR_VAR "Hello, local World!"
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add a new task
    if ! "${BINDIR}"/crinit-ctl addtask "${SMOKETESTS_CONFDIR}"/smoketest_incl.crinit; then
        echo "crinit-ctl addtask failed unexpectedly"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q smoketest_incl; then
        echo "crinit-ctl addtask successful, but task not in list"
        return 1
    fi
 
    sleep 1
    
    if ! "$BINDIR"/crinit-ctl status smoketest_incl | grep -q loaded; then
        echo "crinit-ctl addtask successful, but task already ran before enabling it"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl enable smoketest_incl; then
        echo "crinit-ctl enable of test task failed"
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

