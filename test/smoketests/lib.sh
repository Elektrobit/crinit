# shellcheck shell=sh
#
# smoketests library functions
#
# This file should be sourced from smoketests to provide
# common functionality for tests.
#

if [ -z "$BINDIR" ]; then
    BINDIR=/usr/bin
fi
if [ -z "$SBINDIR" ]; then
    SBINDIR=/sbin
fi
if [ -z "$CONFDIR" ]; then
    CONFDIR="$CMDPATH"/config/test
fi
if [ -z "$SMOKETESTS_CONFDIR" ]; then
    SMOKETESTS_CONFDIR=/tmp/crinit-smoketests-config
fi
if [ -z "$SMOKETESTS_RESULTDIR" ]; then
    SMOKETESTS_RESULTDIR=/tmp/crinit-smoketests
fi
if [ -z "$CRINIT_SOCK" ]; then
    export CRINIT_SOCK=/tmp/crinit.sock
fi

CRINIT_PID=

crinit_config_setup() {
    rm -rf "$SMOKETESTS_CONFDIR"
    mkdir -p "$SMOKETESTS_CONFDIR"
    cp -R "$CONFDIR"/* "$SMOKETESTS_CONFDIR"/
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}#" < "${SMOKETESTS_CONFDIR}"/local.series > "${SMOKETESTS_CONFDIR}"/demo.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add_dyn.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd_dyn.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add_dyn_nofollow.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd_dyn_nofollow.series
    sed "s#TASKDIR = .*#TASKDIR = ${SMOKETESTS_CONFDIR}/addseries#" < "${SMOKETESTS_CONFDIR}"/addseries/add_dyn_suffix.series > "${SMOKETESTS_CONFDIR}"/addseries/demoadd_dyn_suffix.series
}

crinit_daemon_start() {
    set -- "${SBINDIR}"/crinit "$@"

    if [ -n "$SMOKETESTS_VALGRIND" ] && [ "$SMOKETESTS_VALGRIND" -eq 1 ]; then
        set -- valgrind --leak-check=full --log-file="${SMOKETESTS_RESULTDIR}/${SMOKETESTS_NAME}-valgrind.log" "$@"
    fi

    # remove stale socket if it exists
    rm -f "$CRINIT_SOCK"

    # shellcheck disable=SC2024
    "$@" > "${SMOKETESTS_RESULTDIR}/${SMOKETESTS_NAME}-crinit.log" 2>&1 &
    CRINIT_PID="$!"

    # give crinit some time to initialize in background
    i=10
    while [ $i -gt 0 ]; do
        if kill -0 "$CRINIT_PID"; then
            if [ -e "$CRINIT_SOCK" ]; then
                echo "Crinit started with PID ${CRINIT_PID}."
                return 0
            fi
        fi
        sleep 1
        : $(( i -= 1 ))
    done

    return 1
}

crinit_daemon_stop() {
    if [ -n "$CRINIT_PID" ]; then
        if ! kill "$CRINIT_PID"; then
            echo "Crinit with PID ${CRINIT_PID} already exited?"
            return 1
        fi
    fi

    return 0
}

compare_output() {
    sample="$1"
    out="$2"
    if ! cmp -s "$sample" "$out"; then
        diff "$sample" "$out" > "${out%.out}".diff
        return 1
    fi
}
