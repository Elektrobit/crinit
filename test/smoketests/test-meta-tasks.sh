# shellcheck shell=bash
#
# smoketest for testing dependency groups (meta-tasks).
#

task_dep1="${SMOKETESTS_CONFDIR}"/meta_tasks_dep1.crinit
task_dep2="${SMOKETESTS_CONFDIR}"/meta_tasks_dep2.crinit
task_dep3="${SMOKETESTS_CONFDIR}"/meta_tasks_dep3.crinit
task_grp="${SMOKETESTS_CONFDIR}"/meta_tasks_grp.crinit
task_final="${SMOKETESTS_CONFDIR}"/meta_tasks_fin.crinit


setup() {
    crinit_config_setup
    rm -f "${task_dep1}"
    rm -f "${task_dep2}"
    rm -f "${task_dep3}"
    rm -f "${task_grp}"
    rm -f "${task_final}"
    cat << EOF > "${task_dep1}"
# Config to test dependency groups (dep group member 1)

NAME = task_dep1

COMMAND[] = /bin/true

DEPENDS = "@ctl:enable"

RESPAWN = NO
EOF
    cat << EOF > "${task_dep2}"
# Config to test dependency groups (dep group member 2)

NAME = task_dep2

COMMAND[] = /bin/true

DEPENDS = "@ctl:enable"

RESPAWN = NO
EOF
    cat << EOF > "${task_dep3}"
# Config to test dependency groups (dep group member 3)

NAME = task_dep3

COMMAND[] = /bin/true

DEPENDS = "@ctl:enable"

RESPAWN = NO
EOF
    cat << EOF > "${task_grp}"
# Config to test dependency groups (dependency group meta-task)

NAME = task_grp

DEPENDS = "task_dep1:wait task_dep2:wait task_dep3:wait"

PROVIDES = "depgrp:wait"

RESPAWN = NO
EOF
    cat << EOF > "${task_final}"
# Config to test dependency groups (dep group member 3)

NAME = task_final

COMMAND[] = /bin/echo Dependency group has been fulfilled.

DEPENDS = "@provided:depgrp"

RESPAWN = NO
EOF
}

crinit_add_task() {
    if ! "${BINDIR}"/crinit-ctl addtask "$1"; then
        echo "crinit-ctl addtask $1 failed unexpectedly."
        return 1
    fi

    if ! "${BINDIR}"/crinit-ctl list | grep -q "$2"; then
        echo "crinit-ctl addtask successful, but task '$2' not in list."
        return 1
    fi
}

crinit_task_check_status() {
    if ! "${BINDIR}"/crinit-ctl status "$1" | grep -E -q "\bStatus: $2"; then
        echo "crinit-ctl status $1 failed or returned an unexpected status."
        return 1
    fi
}

crinit_enable_task() {
    if ! "${BINDIR}"/crinit-ctl enable "$1"; then
        echo "crinit-ctl enable $1 failed unexpectedly."
        return 1
    fi
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add dependency tasks
    if ! crinit_add_task "${task_dep1}" "task_dep1"; then
        return 1
    fi
    if ! crinit_add_task "${task_dep2}" "task_dep2"; then
        return 1
    fi
    if ! crinit_add_task "${task_dep3}" "task_dep3"; then
        return 1
    fi

    # add dependency group
    if ! crinit_add_task "${task_grp}" "task_grp"; then
        return 1
    fi

    # add final/dependent task
    if ! crinit_add_task "${task_final}" "task_final"; then
        return 1
    fi

    sleep 1

    # check that the final task is loaded but not yet started
    if ! crinit_task_check_status "task_final" "loaded"; then
        return 1
    fi

    # Successively enable dependency tasks and check that final task remains loaded until the last one.
    if ! crinit_enable_task "task_dep1"; then
        return 1
    fi

    sleep 1

    if ! crinit_task_check_status "task_final" "loaded"; then
        return 1
    fi

    if ! crinit_enable_task "task_dep2"; then
        return 1
    fi

    sleep 1

    if ! crinit_task_check_status "task_final" "loaded"; then
        return 1
    fi

    if ! crinit_enable_task "task_dep3"; then
        return 1
    fi

    # Now check that both the dependency group meta-task and the final task are done.
    sleep 1

    if ! crinit_task_check_status "task_grp" "done"; then
        return 1
    fi
    if ! crinit_task_check_status "task_final" "done"; then
        return 1
    fi
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

