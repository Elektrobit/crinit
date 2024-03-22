# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for task timestamps.
#

task_tst1="${SMOKETESTS_CONFDIR}"/task_tst1.crinit
task_tst2="${SMOKETESTS_CONFDIR}"/task_tst2.crinit

setup() {
    crinit_config_setup
    rm -f "${task_tst1}"
    rm -f "${task_tst2}"
    cat << EOF > "${task_tst1}"
# Config to test timestamping (task will be loaded earlier but run later than task_tst2)

NAME = task_tst1
COMMAND = /bin/true
DEPENDS = "@ctl:enable"
EOF
    cat << EOF > "${task_tst2}"
# Config to test timestamping (task will be loaded later but run earlier than task_tst1)

NAME = task_tst2
COMMAND = /bin/true
DEPENDS = "@ctl:enable"
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add timestamped task, first tst1, then tst2
    if ! crinit_add_task "${task_tst1}" "task_tst1"; then
        return 1
    fi

    sleep 1

    if ! crinit_add_task "${task_tst2}" "task_tst2"; then
        return 1
    fi

    sleep 1

    # check if "not yet happened" condition is handled correctly, i.e. 'n/a' is printed
    TST1_STATUS=$("${BINDIR}"/crinit-ctl status "task_tst1")
    TST1_STIME=$(echo "${TST1_STATUS}" | cut -d ' ' -f  8)
    if [ "${TST1_STIME}" != "n/a" ]; then
        echo "A timestamp that has not yet been created unexpectedly shows a value."
        return 1;
    fi

    # run timestamped tasks, first tst2, then tst1
    if ! crinit_enable_task "task_tst2"; then
        return 1
    fi

    sleep 1

    if ! crinit_enable_task "task_tst1"; then
        return 1
    fi

    sleep 2

    # get recorded timestamps
    TST1_STATUS=$("${BINDIR}"/crinit-ctl status "task_tst1")
    TST2_STATUS=$("${BINDIR}"/crinit-ctl status "task_tst2")

    TST1_CTIME=$(echo "${TST1_STATUS}" | cut -d ' ' -f  6 | tr -d 's')
    TST1_STIME=$(echo "${TST1_STATUS}" | cut -d ' ' -f  8 | tr -d 's')
    TST1_ETIME=$(echo "${TST1_STATUS}" | cut -d ' ' -f 10 | tr -d 's')

    TST2_CTIME=$(echo "${TST2_STATUS}" | cut -d ' ' -f  6 | tr -d 's')
    TST2_STIME=$(echo "${TST2_STATUS}" | cut -d ' ' -f  8 | tr -d 's')
    TST2_ETIME=$(echo "${TST2_STATUS}" | cut -d ' ' -f 10 | tr -d 's')

    # check plausibility of recorded timestamps
    if [ "$(echo "${TST1_CTIME} < ${TST1_STIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst1 CTIME > STIME."
        return 1
    fi
    if [ "$(echo "${TST1_STIME} < ${TST1_ETIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst1 STIME > ETIME."
        return 1
    fi

    if [ "$(echo "${TST2_CTIME} < ${TST2_STIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst2 CTIME > STIME."
        return 1
    fi
    if [ "$(echo "${TST2_STIME} < ${TST2_ETIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst2 STIME > ETIME."
        return 1
    fi

    if [ "$(echo "${TST1_CTIME} < ${TST2_CTIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst1 CTIME > task_tst2 CTIME."
        return 1
    fi
    if [ "$(echo "${TST2_STIME} < ${TST1_STIME}" | bc)" -ne "1" ]; then
        echo "Implausible timestamp detected: task_tst2 STIME > task_tst1 STIME."
        return 1
    fi

    return 0
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

