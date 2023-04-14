# shellcheck shell=bash
#
# smoketest for testing IO redirection with a named pipe between two tasks.
#

task_send="${SMOKETESTS_CONFDIR}"/io_redirections_test_pipe_send.crinit
task_recv="${SMOKETESTS_CONFDIR}"/io_redirections_test_pipe_recv.crinit
out_recv="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-stdin-out.txt
pipe_path="${SMOKETESTS_CONFDIR}"/"${SMOKETESTS_NAME}"-fifo
sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.txt

setup() {
    crinit_config_setup
    rm -f "${task_send}"
    rm -f "${task_recv}"
    rm -f "${out_recv}"
    rm -f "${pipe_path}"
    cat << EOF > "${task_send}"
# Config to test IO redirection of STDOUT to a pipe.

NAME = io_redirections_test_pipe_send
COMMAND[] = /bin/echo "56/2 + 7*2"
IO_REDIRECT = STDOUT "${pipe_path}" PIPE 0640
DEPENDS = ""
RESPAWN = NO
EOF
    cat << EOF > "${task_recv}"
# Config to test IO redirection of STDIN from a pipe.

NAME = io_redirections_test_pipe_recv
COMMAND[] = /usr/bin/bc
IO_REDIRECT = STDIN "${pipe_path}" PIPE 0640
IO_REDIRECT = STDOUT "${out_recv}"
DEPENDS = ""
RESPAWN = NO
EOF
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add send and receive tasks
    if ! "${BINDIR}"/crinit-ctl addtask "${task_recv}"; then
        echo "crinit-ctl addtask failed unexpectedly (receive)"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q io_redirections_test_pipe_recv; then
        echo "crinit-ctl addtask successful, but task not in list (receive)"
        return 1
    fi

    if ! "${BINDIR}"/crinit-ctl addtask "${task_send}"; then
        echo "crinit-ctl addtask failed unexpectedly (send)"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q io_redirections_test_pipe_send; then
        echo "crinit-ctl addtask successful, but task not in list (send)"
        return 1
    fi

    sleep 1
    # check output of receiver
    compare_output "${sample}" "${out_recv}"

    # check type and permissions of named pipe
    if ! ls -l "${pipe_path}" | grep -q '\prw-r-----'; then
        echo "Permissions and/or type of FIFO special file are wrong."
        return 1
    fi
    return 0
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

