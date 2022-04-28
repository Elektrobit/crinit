#!/bin/bash -e
#
# project demo script
#

cleanup() {
    # Terminate Crinit on exit.
    sudo kill ${@}
    # Delete temporary config.
    rm -f ${CONFDIR}/demo.series
    rm -f ${CONFDIR}/addseries/demoadd.series
    rm -f ${CONFDIR}/crinit_recursive.crinit
}

CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}
BINDIR=${BASEDIR}/result/bin/x86_64
LIBDIR=${BASEDIR}/result/lib/x86_64
CONFDIR=${BASEDIR}/config/test
export LD_LIBRARY_PATH="${LIBDIR}"

# Remove the socket if we have run crinit before during this docker session.
sudo rm -f /run/crinit/crinit.sock

cd $BASEDIR

cat config/test/local.series | sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}#" > ${CONFDIR}/demo.series
cat config/test/addseries/add.series | sed "s#TASKDIR = .*#TASKDIR = ${CONFDIR}/addseries#" > ${CONFDIR}/addseries/demoadd.series

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
echo "sleep_one_day should now have exited with an error code. We can check by querying its status and look for \"failed\""
echo Will run: $ crinit-ctl status sleep_one_day
${BINDIR}/crinit-ctl status sleep_one_day

echo ""
echo "As an example for the sd_notify API, we can (falsely) tell Crinit sleep_one_day is actually still running as PID 666."
echo Will run: $ sudo crinit-ctl notify sleep_one_day '"$(echo -ne "READY=1\nMAINPID=666")"'
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl notify sleep_one_day "$(echo -ne "READY=1\nMAINPID=666")"

echo ""
echo "We can check if it has worked, it should now read as \"Status: running, PID: 666\"."
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
echo "If there was no or just one more output from one_second_respawn, all is well."

echo ""
echo Now we\'ll try out the RESPAWN_RETRIES functionality, the fail_loop task should not be respawned after 5 unsuccessful \
	retries.
echo Will run: $ sudo crinit-ctl enable fail_loop
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl enable fail_loop
sleep 1
echo "There should be six consecutive outputs from fail_loop now (1 initial run + 5 respawns)."

echo ""
echo Now we\'ll try loading crinit through a crinit task which should work for crinit-ctl but the new crinit should \
    detect the already used socket and fail.
sed -e "s:\${BINDIR}:${BINDIR}:g" -e "s:\${CONFDIR}:${CONFDIR}:g" ${CONFDIR}/crinit_recursive.crinit.in \
    > ${CONFDIR}/crinit_recursive.crinit
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/crinit_recursive.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/crinit_recursive.crinit
sleep 1

echo ""
echo The status should show that crinit-recursive has failed.
echo Will run: $ crinit-ctl status crinit_recursive
CRINIT_REC_STATUS=$(${BINDIR}/crinit-ctl status crinit_recursive)
echo $CRINIT_REC_STATUS

# Use sh magic to check if the output begins with "Status: failed,"
if case $CRINIT_REC_STATUS in "Status: failed,"*) ;; *) false;; esac; then
    echo Status is reported to be "failed" as expected.
else
    echo Status is not "failed" which is unexpected.
    exit 1
fi

echo ""
echo Now we\'ll try loading a second series file containing 3 tasks, forming a chain of dependencies. Three lines \
    should be echo\'d, one after the other.
echo ""
echo Will run: $ sudo crinit-ctl addseries ${CONFDIR}/addseries/add.series
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addseries ${CONFDIR}/addseries/demoadd.series
sleep 1

echo ""
echo Now we\'ll check parsing of correct and incorrect COMMAND chains in the config files. We will try to load 6 task \
    configurations, 2 of which are correct and 4 of which shall cause an error during parsing/loading.

echo ""
echo The first task config is correctly using empty brackets and should result in the numbers 0-4 to be printed in \
    separate lines.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_ebrk.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_ebrk.crinit
sleep 1

echo ""
echo The second task config is correctly using numbered brackets and should result in the numbers 0-4 to be printed in \
    separate lines.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_num.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_num.crinit
sleep 1

echo ""
echo The third task config is incorrectly using empty brackets and should result in an error.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_err1.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_err1.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Crinit-ctl reported an error as expected."
else
    echo "Crinit-ctl reported no error which is unexpected."
    exit 1
fi

echo ""
echo The fourth task config is incorrectly using a mix of empty and numbered brackets and should result in an error.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_err2.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_err2.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Crinit-ctl reported an error as expected."
else
    echo "Crinit-ctl reported no error which is unexpected."
    exit 1
fi

echo ""
echo The fifth task config is incorrectly using numbered brackets \(with a duplicate\) and should result in an error.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_err3.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_err3.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Crinit-ctl reported an error as expected."
else
    echo "Crinit-ctl reported no error which is unexpected."
    exit 1
fi

echo ""
echo The sixth task config is incorrectly using numbered brackets \(with a missing number\) and should result in an \
    error.
echo Will run: $ sudo crinit-ctl addtask ${CONFDIR}/chain_err4.crinit
sudo LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ${BINDIR}/crinit-ctl addtask ${CONFDIR}/chain_err4.crinit || RETURNCODE=$?
if [ "$RETURNCODE" -ne "0" ]; then
    echo "Crinit-ctl reported an error as expected."
else
    echo "Crinit-ctl reported no error which is unexpected."
    exit 1
fi

echo ""
echo "All commands/checks returned as expected."
exit 0
