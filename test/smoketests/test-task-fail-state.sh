# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for handling of task failure states.
#

task_fail1="${SMOKETESTS_CONFDIR}"/task_fail1.crinit
task_fail2="${SMOKETESTS_CONFDIR}"/task_fail2.crinit
task_fail3="${SMOKETESTS_CONFDIR}"/task_fail3.crinit
task_dependent="${SMOKETESTS_CONFDIR}"/task_fail_dependent.crinit
out_stdout="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-stdout-out.txt
out_sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.txt

setup() {
    crinit_config_setup
    rm -f "${task_fail1}"
    rm -f "${task_fail2}"
    rm -f "${task_dependent}"
    rm -f "${out_stdout}"
    cat << EOF > "${task_fail1}"
# Config to test handling of task failures (bad return code)

NAME = task_fail1
COMMAND = /bin/false
          /bin/echo This should not be printed.
DEPENDS = "@ctl:enable"

IO_REDIRECT = STDOUT "${out_stdout}"
EOF
    cat << EOF > "${task_fail2}"
# Config to test handling of task failures (command not found)

NAME = task_fail2
COMMAND = /bin/this/does/not/exist
DEPENDS = "@ctl:enable"
EOF
    cat << EOF > "${task_fail3}"
# Config to test handling of task failures (command is not an absolute path)

NAME = task_fail3
COMMAND = true
DEPENDS = "@ctl:enable"
EOF
    cat << EOF > "${task_dependent}"
# Config to test handling of task failures (dependent task to check if the others have failed.)

NAME = task_dependent
COMMAND = /bin/echo This should be printed.
DEPENDS = task_fail1:fail task_fail2:fail task_fail3:fail

IO_REDIRECT = STDOUT "${out_stdout}" APPEND
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add failing tasks
    if ! crinit_add_task "${task_fail1}" "task_fail1"; then
        return 1
    fi
    if ! crinit_add_task "${task_fail2}" "task_fail2"; then
        return 1
    fi
    if ! crinit_add_task "${task_fail3}" "task_fail3"; then
        return 1
    fi

    # add dependent task
    if ! crinit_add_task "${task_dependent}" "task_dependent"; then
        return 1
    fi

    sleep 1

    # check that the final task is loaded but not yet started
    if ! crinit_task_check_status "task_dependent" "loaded"; then
        return 1
    fi

    # Successively enable dependency tasks and check that dependent task remains loaded until the last one.
    if ! crinit_enable_task "task_fail1"; then
        return 1
    fi

    sleep 1

    if ! crinit_task_check_status "task_dependent" "loaded"; then
        return 1
    fi

    if ! crinit_enable_task "task_fail2"; then
        return 1
    fi

    sleep 1

    if ! crinit_task_check_status "task_dependent" "loaded"; then
        return 1
    fi

    if ! crinit_enable_task "task_fail3"; then
        return 1
    fi

    sleep 4

    # Now check states
    if ! crinit_task_check_status "task_dependent" "done"; then
        return 1
    fi
    if ! crinit_task_check_status "task_fail1" "failed"; then
        return 1
    fi
    if ! crinit_task_check_status "task_fail2" "failed"; then
        return 1
    fi
    if ! crinit_task_check_status "task_fail3" "failed"; then
        return 1
    fi

    # check output(s)
    compare_output "${out_sample}" "${out_stdout}"

    return 0
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

