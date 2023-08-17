# shellcheck shell=bash
# SPDX-License-Identifier: MIT
#
# smoketest for testing the parser of COMMANDs including quoting and escaping.
#

task_stdout="${SMOKETESTS_CONFDIR}"/io_redirections_test_stdout.crinit
task_stderr="${SMOKETESTS_CONFDIR}"/io_redirections_test_stderr.crinit
task_stdin="${SMOKETESTS_CONFDIR}"/io_redirections_test_stdin.crinit
in_stdin="${SMOKETESTS_CONFDIR}"/io_redirections_test_stdin-in.txt
out_stdout="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-stdout-out.txt
out_stderr="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-stderr-out.txt
out_stdin="${SMOKETESTS_RESULTDIR}"/"${SMOKETESTS_NAME}"-stdin-out.txt
sample="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample.txt
sample_stdin="${CMDPATH}"/test-"${SMOKETESTS_NAME}"-sample-stdin.txt

setup() {
    crinit_config_setup
    rm -f "${out_stdout}"
    rm -f "${out_stderr}"
    rm -f "${out_stdin}"
    rm -f "${task_stdout}"
    rm -f "${task_stderr}"
    rm -f "${task_stdin}"
    cat << EOF > "${task_stdout}"
# Config to test IO redirection of STDOUT

NAME = io_redirections_test_stdout

COMMAND[] = /bin/echo " '''   \\xe2\\x99\\xa5 "\\x68ello w\\ spc

IO_REDIRECT = STDOUT "${out_stdout}"

DEPENDS = ""

RESPAWN = NO
EOF
    cat << EOF > "${task_stderr}"
# Config to test a chain of IO redirections from STDOUT to STDERR in shell, then in Crinit back to STDOUT and to file.
# Also tests setting the file mode.

NAME = io_redirections_test_stderr

COMMAND[] = /bin/sh -c "echo \\" '''   \\xe2\\x99\\xa5  \\"\\x68ello w\\ spc 1>&2"

IO_REDIRECT = STDOUT "${out_stderr}" TRUNCATE 0640
              STDERR STDOUT

DEPENDS = ""

RESPAWN = NO
EOF
    cat << EOF > "${task_stdin}"
# Config to test a redirection from a file to STDIN.
# Also tests appending to the output file.

NAME = io_redirections_test_stdin

COMMAND[] = /usr/bin/tee

IO_REDIRECT = STDOUT "${out_stdin}" APPEND
IO_REDIRECT = STDIN "${in_stdin}" 

DEPENDS = ""

RESPAWN = NO
EOF
echo -n "prefix: " > ${out_stdin}
echo "suffix" > ${in_stdin}
}

run() {
    crinit_daemon_start "${SMOKETESTS_CONFDIR}"/demo.series
    sleep 3

    # add stdout redirection task
    if ! "${BINDIR}"/crinit-ctl addtask "${task_stdout}"; then
        echo "crinit-ctl addtask failed unexpectedly (stdout)"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q io_redirections_test_stdout; then
        echo "crinit-ctl addtask successful, but task not in list (stdout)"
        return 1
    fi

    sleep 1
    compare_output "${sample}" "${out_stdout}"
   
    # add stderr/chain redirection task
    if ! "${BINDIR}"/crinit-ctl addtask "${task_stderr}"; then
        echo "crinit-ctl addtask failed unexpectedly (stderr/chain)"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q io_redirections_test_stderr; then
        echo "crinit-ctl addtask successful, but task not in list (stderr/chain)"
        return 1
    fi

    sleep 1
    compare_output "${sample}" "${out_stderr}"
    # check permissions
    if ! ls -l "${out_stderr}" | grep -q '\-rw-r-----'; then
        echo "Permissions of stderr output file are wrong."
        return 1
    fi

    # add stdin redirection task with append
    if ! "${BINDIR}"/crinit-ctl addtask "${task_stdin}"; then
        echo "crinit-ctl addtask failed unexpectedly (stdin)"
        return 1
    fi

    if ! "$BINDIR"/crinit-ctl list | grep -q io_redirections_test_stdin; then
        echo "crinit-ctl addtask successful, but task not in list (stdin)"
        return 1
    fi

    sleep 1
    compare_output "${sample_stdin}" "${out_stdin}"
}

teardown() {
    # Terminate crinit daemon
    crinit_daemon_stop
}

