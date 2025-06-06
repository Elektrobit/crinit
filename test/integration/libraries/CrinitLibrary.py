# SPDX-License-Identifier: MIT

import io
import os
import re
import traceback

from robot.libraries.BuiltIn import BuiltIn
from robot.utils.asserts import assert_equal, assert_not_equal
from robot.api import logger
from robot.api.deco import keyword
from ctypes import cdll, c_char_p, c_int, POINTER


class CrinitLibrary(object):
    """CrinitLibrary provides a set of keywords for direct interaction
    with target crinit-ctl.
    """

    CRINIT_BIN = "/usr/bin/crinit"
    CRINIT_SERIES = "/etc/crinit/itest/itest.series"
    WAIT_TIMEOUT = 30

    def __init__(self):
        self.LOCAL_TEST_DIR = BuiltIn().get_variable_value('${LOCAL_TEST_DIR}')
        self.IS_ROOT = (lambda s: True if s == "True" else False)(BuiltIn().get_variable_value('${USER_IS_ROOT}'))
        self.CRINIT_SOCK = BuiltIn().get_variable_value('${CRINIT_SOCK}')
        self.ssh = BuiltIn().get_library_instance('SSHLibrary')
        self.password = BuiltIn().get_variable_value('${PASSWORD}')
        self.crinit = None

    def __print_traceback(self):
        logger.error("Exception in user code:")
        logger.error("-" * 60)
        logger.error(traceback.format_exc())
        logger.error("-" * 60)

    def __local_path_service(self, file):
        return os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)) + "/../service", file))

    def __local_path(self, file):
        return os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), file))

    def __target_path(self, file):
        return os.path.join(self.LOCAL_TEST_DIR, os.path.basename(file))

    def __crinit_run(self, action, err_msg, **kwargs):
        """Will run a crinit-ctl action.

        | action | A crinit-ctl action name |
        | err_msg | An error message to be displayed on error |
        | task | The related task name or task configuration path (optional) |
        | options | crinit-ctl options (optional) |
        """
        stdout = None
        stderr = None
        ret = -1

        stdout, stderr, ret = self.ssh.execute_command(f"sh -c \"export CRINIT_SOCK={self.CRINIT_SOCK}; \
                    crinit-ctl {action} {kwargs.get('task') or ''} {kwargs.get('options') or ''}\"",
                                                       return_stdout=True,
                                                       return_stderr=True,
                                                       return_rc=True,
                                                       sudo=(not self.IS_ROOT),
                                                       sudo_password=(None if self.IS_ROOT else self.password))

        logger.info(f"{stdout}")
        if ret != 0 and stderr:
            logger.error(f"crinit-ctl: {err_msg} ({ret}): {stderr}")
        elif ret != 0:
            logger.error(f"crinit-ctl: {err_msg} ({ret})")

        return (stdout, stderr, ret)

    @keyword("A Task Config ${task_config}")
    def crinit_create_task_config(self, task_config):
        # conf = BuiltIn().get_variable_value("${task_config}")
        task_config = task_config.replace('@@CAP_SET_KEY@@', BuiltIn().get_variable_value("${CAP_SET_KEY}"))
        task_config = task_config.replace('@@CAP_SET_VAL@@', BuiltIn().get_variable_value("${CAP_SET_VAL}"))
        task_config = task_config.replace('@@CAP_CLEAR_KEY@@', BuiltIn().get_variable_value("${CAP_CLEAR_KEY}"))
        task_config = task_config.replace('@@CAP_CLEAR_VAL@@', BuiltIn().get_variable_value("${CAP_CLEAR_VAL}"))
        logger.info(f"New task config:\n{task_config}")

        BuiltIn().set_test_variable("${TASK_CONFIG}", task_config)

    @keyword("Crinit Starts The ${task} With ${task_config}")
    def crinit_starts_the_task_with_config(self, task, new_conf):
        rc = self.crinit_add_task_config(task, new_conf)
        assert_equal(rc, 0, "assert: failed to add task config")
        if rc != 0:
            logger.error(f"Failed to add crinit task config {new_conf}.")
            return rc

        rc = self.crinit_add_task(f"{task}.crinit")
        assert_equal(rc, 0, "assert: failed to add task")
        if rc != 0:
            logger.error(f"Failed to add crinit task {task}.")
            return rc

        return 0

    @keyword("Crinit Task Creation Was '${exp_task_creation}'")
    def crinit_task_is_created_successfully(self, exp_task_creation):
        task = BuiltIn().get_variable_value("${TASK}")
        rc, task_state = self._crinit_get_task_state(task)

        if (exp_task_creation == "Successful"):
            assert_equal(task_state, "running", "Task is running")
        else:
            assert_not_equal(task_state, "running", "Task is not running")

    def crinit_add_task_config(self, task, config):
        """ Creates a new crinit task config on the target.

        | task | The crinit task name |
        | config | The crinit config |
        """
        local_path = self.__local_path(f"{task}.crinit")
        target_path = self.__target_path(f"{task}.crinit")

        with open(local_path, "w") as f:
            f.write(config)

        try:
            self.ssh.put_file(local_path, target_path, scp="ALL")
        except Exception:
            self.__print_traceback()
            return -1

        return 0

    def crinit_is_running(self):
        """ Checks if crinit is already running at the specified socket. """
        ret = -1

        stdout, ret = self.ssh.execute_command(f"test -S {self.CRINIT_SOCK}", return_rc=True)

        return ret == 0

    def configure_default_capabilities(self, series_file, cap_default_val):
        logger.info(f"Add default capability to .series config: {cap_default_val}")

        local_path_series_config = self.__local_path_service(os.path.basename(series_file))
        config_data = ""
        with open(local_path_series_config, "r") as f:
            config_data = f.read()
        config_data = config_data + f"\nDEFAULTCAPS = {cap_default_val}\n"

        target_path_series_config = self.__target_path(os.path.basename(series_file))
        with open(target_path_series_config, "w") as f:
            f.write(config_data)

        try:
            self.ssh.put_file(target_path_series_config, target_path_series_config, scp="ALL")
        except Exception:
            self.__print_traceback()
            return -1

        return target_path_series_config

    def crinit_start(self,
                     series_file=None,
                     chroot=None,
                     strace_output=None,
                     strace_filter=None,
                     ld_preload=None,
                     crinit_args=None,
                     cap_default_val=None):
        """ Starts crinit if it is not already running with the specified socket. """
        chroot_cmd = ""
        strace_cmd = ""
        ldprld_cmd = ""
        if chroot is not None:
            chroot_cmd = f"chroot {chroot}"
        if strace_output is not None:
            strace_cmd = f"strace --output={strace_output}"
        if strace_filter is not None:
            strace_cmd = f"{strace_cmd} --trace={strace_filter}"
        if series_file is None:
            series_file = self.CRINIT_SERIES
        if ld_preload is not None:
            ldprld_cmd = f"export LD_PRELOAD=/etc/crinit/itest/ld_preload/{ld_preload};"
        if cap_default_val is not None:
            series_file = self.configure_default_capabilities(series_file, cap_default_val)
        self.eff_series_file = series_file

        if self.crinit_is_running():
            return 0

        start_cmd = \
            f"sh -c \"export CRINIT_SOCK={self.CRINIT_SOCK}; {ldprld_cmd} {chroot_cmd} {strace_cmd} {self.CRINIT_BIN}"
        if crinit_args is not None:
            start_cmd = start_cmd + f" {crinit_args}"
        start_cmd = start_cmd + f" {self.eff_series_file}\""

        self.ssh.start_command(start_cmd,
                               sudo=(not self.IS_ROOT),
                               sudo_password=(None if self.IS_ROOT else self.password))

        try:
            self.ssh.execute_command(f"sh -c \"until [ -S {self.CRINIT_SOCK} ]; do sleep 0.1; done\"",
                                     sudo=(not self.IS_ROOT),
                                     sudo_password=(None if self.IS_ROOT else self.password),
                                     timeout=self.WAIT_TIMEOUT)
        except Exception as e:
            out, err = self.ssh.read_command_output(return_stderr=True, timeout=self.WAIT_TIMEOUT)
            logger.info(out)
            logger.error(err)
            raise e

        return 0

    @keyword("Crint Start With Default Capabilities ${cap_default_val}")
    def crinit_start_with_default_caps(self, cap_default_val):
        return self.crinit_start(None, None, None, None, None, None, cap_default_val)

    def crinit_stop(self, crinit_args=None):
        """ Stops all remaining crinit tasks and kills crinit. """

        if not self.crinit_is_running():
            return 0

        res = self.crinit_kill_all_tasks()
        if res != 0:
            logger.error("Failed to kill all running tasks.")
            return res

        ret = -1

        stop_cmd = f"sh -c \"pkill -f {self.CRINIT_BIN}"
        if crinit_args is not None:
            stop_cmd = stop_cmd + f"\\ {crinit_args}"
        stop_cmd = stop_cmd + f"\\ {self.eff_series_file} && rm -rf {self.CRINIT_SOCK}\""

        stdout, stderr, ret = self.ssh.execute_command(stop_cmd,
                                                       return_stdout=True,
                                                       return_stderr=True,
                                                       return_rc=True,
                                                       sudo=(not self.IS_ROOT),
                                                       sudo_password=(None if self.IS_ROOT else self.password))

        if stdout:
            logger.info(stdout)
        if ret != 0 and stderr:
            logger.error(stderr)

        assert ret == 0

    def crinit_add_task(self, task_config):
        """Will add a task defined in the task configuration file
        at <task_config> (absolute) to Crinit's task database.

        | task_config | The crinit task file |
        """
        target_path = self.__target_path(task_config)

        return self.__crinit_run("addtask", "Adding task failed", task=target_path, options="-f")[2]

    def crinit_enable_task(self, task_name):
        """Removes dependency '@ctl:enable' from the dependency list of <task_name> if it is present.

        | task_name | The task name |
        """
        return self.__crinit_run("enable", "Enabling task failed", task=task_name)[2]

    def crinit_disable_task(self, task_name):
        """Adds dependency '@ctl:enable' to the dependency list of <task_name>.

        | task_name | The task name |
        """
        return self.__crinit_run("disable", "Disabling task failed", task=task_name)[2]

    def crinit_stop_task(self, task_name):
        """Sends SIGTERM to the PID of <task_name> if the PID is currently known.

        | task_name | The task name |
        """
        return self.__crinit_run("stop", "Stopping task failed", task=task_name)[2]

    def crinit_kill_task(self, task_name):
        """Sends SIGKILL to the PID of <task_name> if the PID is currently known.

        | task_name | The task name |
        """
        return self.__crinit_run("kill", "Killing task failed", task=task_name)[2]

    """ for crinit-ctl ls proc ; do PID != -1 -> Kill process; done Finally kill crinit """

    def crinit_kill_all_tasks(self):
        """ Kills all running crinit tasks and stops crinit """
        (stdout, stderr, ret) = self.__crinit_run("list", "Could not list tasks.")

        if ret != 0:
            logger.error("Failed to list active crinit tasks.")
            return ret

        for line in io.StringIO(stdout):
            # Skip output header
            if line.startswith("NAME"):
                continue

            task_name, task_pid = line.split()[:2]

            # Skip stopped processes
            if task_pid == "-1":
                continue

            res = self.crinit_disable_task(task_name)
            if res != 0:
                logger.error(f"Failed to disable task {task_name}")
                return res

            res = self.crinit_kill_task(task_name)
            if res != 0:
                logger.error(f"Failed to kill task {task_name}")
                return res

        return 0

    def crinit_restart_task(self, task_name):
        """Resets the status bits of <task_name> if it is DONE or FAILED.

        | task_name | The task name |
        """
        return self.__crinit_run("restart", "Restarting task failed", task=task_name)[2]

    def _crinit_get_task_state(self, task_name):
        stdout, stderr, ret = self.__crinit_run("status", "Requesting task status failed", task=task_name)
        if ret != 0:
            return -1, ""

        match = re.search(r"(?i)\bstatus\b\s*:\s*(?P<state>\w+)\s*,\s*\bpid\b\s*:\s*(?P<pid>[\+-]?\d+)", stdout)
        if match is None:
            logger.error(f"Failed to parse crinit state for task {task_name}.")
            return -1, ""

        task_state = match.groupdict()
        return 0, task_state['state']

    def crinit_check_task_state(self, task_name, state):
        """Queries status bits and PID of <task_name>.

        | task_name | The task name |
        """
        res, task_state = self._crinit_get_task_state(task_name)
        if res != 0:
            return res

        return 0 if task_state == state else -1

    def __crinit_get_task_pid(self, task_name):
        """Queries the  PID of <task_name>.

        | task_name | The task name |
        """
        stdout, stderr, ret = self.__crinit_run("status", "Requesting task status failed", task=task_name)
        if ret != 0:
            logger.error(f"Failed requet status of task {task_name}")
            return -1

        match = re.search(r"(?i)\bstatus\b\s*:\s*(?P<state>\w+)\s*,\s*\bpid\b\s*:\s*(?P<pid>[\+-]?\d+)", stdout)
        if match is None:
            logger.error(f"Failed to parse crinit state for task {task_name}.")
            return -1

        pid = int(match.groupdict()['pid'])
        logger.debug(f"pid of task {task_name}: {pid}")

        return pid

    @keyword("'${task}' User Is '${user}' And Group Is '${group}'")
    def task_user_and_group_are(self, task, user, group):
        """
        Check if task user is given user and group is given group
        """

        pid = self.__crinit_get_task_pid(task)

        stdout = None
        stderr = None
        ret = -1

        stdout, stderr, ret = self.ssh.execute_command(f"ps -p{pid} -ouser=,group=",
                                                       return_stdout=True,
                                                       return_stderr=True,
                                                       return_rc=True,
                                                       sudo=False,
                                                       sudo_password=None)

        (rcuser, rcgroup) = stdout.split()
        logger.info(f"USER: {rcuser}")
        logger.info(f"GROUP: {rcgroup}")

        if rcuser == user and rcgroup == group:
            return 0

        return -1

    # int cap_from_name(const char* name , cap_value_t* cap_p);
    def __c_wrapper_cap_from_name(self, cap_name):
        libcap_hdl = cdll.LoadLibrary('libcap.so.2')
        libcap_hdl.cap_from_name.argtypes = [c_char_p, POINTER(c_int)]
        libcap_hdl.cap_from_name.restype = c_int

        cap_val = c_int()

        ret = libcap_hdl.cap_from_name(cap_name.encode('utf8'), cap_val)
        assert_equal(ret, 0, msg=f"Failed to convert capability name ${cap_name}")

        return 1 << cap_val.value

    def __assemble_cap_bitmask(self, cap_names):
        bitmask = 0

        for cap in cap_names.split():
            bitmask |= self.__c_wrapper_cap_from_name(cap)

        logger.debug(f"Convert {cap_names} into bitmask {hex(bitmask)}.")

        return bitmask

    def __get_eff_cap_from_proc(self, pid):
        stdout = None
        stderr = None
        ret = -1
        cap_eff_line = 0

        stdout, stderr, ret = self.ssh.execute_command(
            f"cat /proc/{pid}/status",
            return_stdout=True,
            return_stderr=True,
            return_rc=True,
            sudo=False,
            sudo_password=None,
        )

        for line in stdout.splitlines():
            if "CapEff" in line:
                # line looks like this: "CapEff:    0000000000001021"
                cap_eff_line = line.partition(":")[2].strip()
                break

        logger.info(f"pid {pid}: Effective capabilities ({hex(int(cap_eff_line, 16))})")

        return int(cap_eff_line, 16)

    @keyword("The '${task}' Should Have The Capabilities '${exp_cap_proc}'")
    def crinit_task_has_capabilities(self, task, exp_cap_proc):
        pid = self.__crinit_get_task_pid(task)
        if pid == -1:
            logger.error(f"Task {task} does not exist.")
            return -1

        assert_equal(
            hex(self.__get_eff_cap_from_proc(pid)),
            hex(self.__assemble_cap_bitmask(exp_cap_proc)),
            msg=f"""Failed to verify configured capabilities {hex(self.__assemble_cap_bitmask(exp_cap_proc))}. CapEff
             from /proc: {hex(self.__get_eff_cap_from_proc(pid))}""",
        )

    @keyword("'${task}' User Is '${user}' And Supplementary Groups Are '${multigroups}'")
    def task_user_and_supgroups_are(self, task, user, multigroups):
        """
        Check if task user is given user and supplementary groups are given groups
        """

        pid = self.__crinit_get_task_pid(task)

        stdout = None
        stderr = None
        ret = -1

        stdout, stderr, ret = self.ssh.execute_command(f"ps -p{pid} -ouser=,group=,supgrp=",
                                                       return_stdout=True,
                                                       return_stderr=True,
                                                       return_rc=True,
                                                       sudo=False,
                                                       sudo_password=None)

        logger.info(f"STDOUT: {stdout}")
        logger.info(f"RET: {ret}")
        tmpstring = re.sub(r'\s+', ' ', stdout)
        (rcuser, rcgroup, tmpsupgroups) = tmpstring.split(' ', 2)
        rcsupgroups = tmpsupgroups.split(',')
        supgroups = multigroups.split()
        logger.info(f"USER: {rcuser}")
        logger.info(f"GROUP: {rcgroup}")
        logger.info(f"SUPGROUPS: {rcsupgroups}")
        group = supgroups.pop(0)
        logger.info(f"Target sup groups: {supgroups}")

        if rcuser == user and rcgroup == group and sorted(rcsupgroups) == sorted(supgroups):
            return 0

        return -1

    def crinit_reboot(self):
        """Will request Crinit to perform a graceful system reboot.
        crinit-ctl can be symlinked to reboot as a shortcut which will
        invoke this command automatically.
        """
        return self.__crinit_run("reboot", "Requesting reboot failed")[2]

    def crinit_poweroff(self):
        """Will request Crinit to perform a graceful system shutdown.
        crinit-ctl can be symlinked to poweroff as a shortcut which will
        invoke this command automatically.
        """
        return self.__crinit_run("poweroff", "Requesting poweroff failed")[2]

    @keyword("The Crinit Task ${task_name} Containing ${task_body} Is Loaded")
    def crinit_task_is_loaded(self, task_name: str, task_body: str):
        assert_equal(self.crinit_add_task_config(task_name, task_body), 0,
                     f"The task {task_name} could not be copied to the target.")
        assert_equal(self.crinit_add_task(f"{task_name}.crinit"), 0, f"The task {task_name} could not be loaded.")

    @keyword("The Task ${task_name} Has Exited Successfully")
    def crinit_task_successful(self, task_name: str):
        assert_equal(self.crinit_check_task_state(task_name, "done"), 0,
                     f"The task {task_name} has not (yet) exited successfully.")

    @keyword("The Task ${task_name} Has Exited Unsuccessfully")
    def crinit_task_unsuccessful(self, task_name: str):
        assert_equal(self.crinit_check_task_state(task_name, "failed"), 0,
                     f"The task {task_name} has not (yet) exited unsuccessfully.")

    @keyword("The Task ${task_name} Is Currently Running")
    def crinit_task_running(self, task_name: str):
        assert_equal(self.crinit_check_task_state(task_name, "running"), 0,
                     f"The task {task_name} is not currently running")

    @keyword("Ensure Regular Files Exist")
    def reg_files_exist(self, *files: os.path):
        for file in files:
            stdout, stderr, ret = self.ssh.execute_command(f"test -f {file}",
                                                           return_stdout=True,
                                                           return_stderr=True,
                                                           return_rc=True,
                                                           sudo=(not self.IS_ROOT),
                                                           sudo_password=(None if self.IS_ROOT else self.password))
            if stdout:
                logger.info(stdout)
            if stderr:
                logger.error(stderr)
            assert_equal(ret, 0)
