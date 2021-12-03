#!/bin/bash -e
#
# project demo script
#

cleanup() {
    # Terminate Crinit on exit.
    sudo kill ${@}
    # Delete temporary config.
    rm -f ${CONFDIR}/demo.series
}

CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}
BINDIR=${BASEDIR}/result/bin/x86_64
LIBDIR=${BASEDIR}/result/lib/x86_64
CONFDIR=${BASEDIR}/config/test
export LD_LIBRARY_PATH="${LIBDIR}"

cd $BASEDIR

cat config/test/local.series | sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}#" > ${CONFDIR}/demo.series

echo Will run: $ sudo crinit ${CONFDIR}/demo.series &
sudo ${BINDIR}/crinit ${CONFDIR}/demo.series &
SUDO_PID="$!"
sleep 1
CRINIT_PID=$(ps -o pid= --ppid ${SUDO_PID})
trap "cleanup ${CRINIT_PID}" EXIT
sleep 1
echo Crinit started with PID ${CRINIT_PID}.

echo ""
echo Now we sleep 3 more seconds, so everything has settled...
sleep 3

echo ""
echo Now we check the status/PID of after_sleep.
echo Will run: $ crinit-ctl status after_sleep
${BINDIR}/crinit-ctl status after_sleep

echo ""
echo Now we check if Crinit lets us do forbidden things. Will try to add a task as an unprivileged user. This should fail.
echo Will run: $ crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit
RETURNCODE=0
${BINDIR}/crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Permission was denied as expected."
else
    echo "Crinit-ctl reported no error which seems wrong."
    exit 1
fi

echo ""
echo Now we try the same thing again but with sudo.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit

echo ""
echo "This should not work a second time (unless we specify -f/--overwrite to overwrite the existing entry in the TaskDB)."
echo "Will run (again): $ sudo crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit"
RETURNCODE=0
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/sleep_one_day.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Crinit-ctl returned an error as expected."
else
    echo "Crinit-ctl reported no error which seems wrong."
    exit 1
fi

sleep 1
echo ""
echo Now Crinit should have a /bin/sleep child process which will sleep a whole day if we let it.
ps --ppid ${CRINIT_PID} ## Will return 1 if no processes found.

echo ""
echo "Now let's check the status of sleep_one_day. As it is currently running, the status should be 2 and the PID the same as in ps above."
echo Will run: $ crinit-ctl status sleep_one_day
${BINDIR}/crinit-ctl status sleep_one_day

echo ""
echo "Now we kill the sleep_one_day task as we do not have that kind of time."
echo Will run: $ sudo crinit-ctl kill sleep_one_day
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl kill sleep_one_day

echo ""
echo "sleep_one_day should now have exited with an error code. We can check by querying its status. Status 8 means \"failed\""
echo Will run: $ crinit-ctl status sleep_one_day
${BINDIR}/crinit-ctl status sleep_one_day

echo ""
echo "As an example for the sd_notify API, we can (falsely) tell Crinit sleep_one_day is actually still running as PID 666."
echo Will run: $ sudo crinit-ctl notify sleep_one_day '"$(echo -ne "READY=1\nMAINPID=666")"'
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl notify sleep_one_day "$(echo -ne "READY=1\nMAINPID=666")"

echo ""
echo "We can check if it has worked, it should now read as \"Status: 2, PID: 666\"."
echo Will run: $ crinit-ctl status sleep_one_day
${BINDIR}/crinit-ctl status sleep_one_day

echo ""
echo "Now we enable the already loaded task one_second_respawn (which depends on @ctl:enable, i.e. is deactivated by default)."
echo "You should see some output for 5 seconds."
echo Will run: $ sudo crinit-ctl enable one_second_respawn
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl enable one_second_respawn
sleep 5

echo ""
echo "Okay, that should be enough, let's disable it again and wait 5 seconds to see if there is more output coming."
echo Will run: $ sudo crinit-ctl disable one_second_respawn
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl disable one_second_respawn
sleep 5

echo ""
echo "If there was no or just one more output from one_second_respawn, all is well. Goodbye."
exit 0

