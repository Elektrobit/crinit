# SPDX-License-Identifier: MIT

import io
import os
import re
import traceback

from robot.libraries.BuiltIn import BuiltIn
from robot.api import logger


class CrinitLibrary(object):
    """CrinitLibrary provides a set of keywords for direct interaction
    with target crinit-ctl.
    """

    CRINIT_BIN = "/sbin/crinit"
    CRINIT_SOCK = "/tmp/crinit-itest.sock"
    WAIT_TIMEOUT = 30

    def __init__(self):
        self.LOCAL_TEST_DIR = BuiltIn().get_variable_value('${LOCAL_TEST_DIR}')
        self.IS_ROOT = (lambda s: True if s == "True" else False)(BuiltIn().get_variable_value('${USER_IS_ROOT}'))
        self.ssh = BuiltIn().get_library_instance('SSHLibrary')
        self.password = BuiltIn().get_variable_value('${PASSWORD}')
        self.crinit = None

    def __print_traceback(self):
        logger.error("Exception in user code:")
        logger.error("-"*60)
        logger.error(traceback.format_exc())
        logger.error("-"*60)

    def __local_path(self, file):
        return os.path.normpath(
            os.path.join(
                os.path.dirname(os.path.realpath(__file__)), file
            )
        )

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

        try:
            stdout, stderr, ret = self.ssh.execute_command(
                f"sh -c \"export CRINIT_SOCK={self.CRINIT_SOCK}; \
                        crinit-ctl {action} {kwargs.get('task') or ''} {kwargs.get('options') or ''}\"",
                return_stdout=True,
                return_stderr=True,
                return_rc=True,
                sudo=(not self.IS_ROOT),
                sudo_password=(None if self.IS_ROOT else self.password)
            )
        except Exception:
            self.__print_traceback()

        if ret != 0 and stderr:
            logger.error(f"crinit-ctl: {err_msg} ({ret}): {stderr}")
        elif ret != 0:
            logger.error(f"crinit-ctl: {err_msg} ({ret})")

        return (stdout, stderr, ret)

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
        """ Checks if crinit is already running. """
        ret = -1

        try:
            stdout, ret = self.ssh.execute_command(
                f"pgrep {self.CRINIT_BIN}",
                return_rc=True
            )
        except Exception:
            self.__print_traceback()

        return ret == 0

    def crinit_start(self):
        """ Starts crinit if it is not running. """
        if self.crinit_is_running():
            return 0

        try:
            self.ssh.start_command(
                f"sh -c \"export CRINIT_SOCK={self.CRINIT_SOCK}; {self.CRINIT_BIN}\"",
                sudo=(not self.IS_ROOT),
                sudo_password=(None if self.IS_ROOT else self.password)
            )
        except Exception:
            self.__print_traceback()
            return -1

        try:
            self.ssh.execute_command(
                f"until [ -S {self.CRINIT_SOCK} ]; do sleep 0.1; done",
                sudo=(not self.IS_ROOT),
                sudo_password=(None if self.IS_ROOT else self.password),
                timeout=self.WAIT_TIMEOUT
            )
        except Exception as e:
            out, err = self.ssh.read_command_output(return_stderr=True, timeout=self.WAIT_TIMEOUT)
            logger.info(out)
            logger.error(err)
            raise e

        return 0

    def crinit_stop(self):
        """ Stops all remaining crinit tasks and kills crinit. """
        if not self.crinit_is_running():
            return 0

        res = self.crinit_kill_all_tasks()
        if res != 0:
            logger.error("Failed to kill all running tasks.")
            return res

        ret = -1
        try:
            stdout, ret = self.ssh.execute_command(
                f"pkill {self.CRINIT_BIN} && rm -rf {self.CRINIT_SOCK}",
                return_rc=True,
                sudo=(not self.IS_ROOT),
                sudo_password=(None if self.IS_ROOT else self.password)
            )
        except Exception:
            self.__print_traceback()

        return ret == 0

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
        return self.__crinit_task_run("kill", "Killing task failed", task=task_name)[2]

    """ for crinit-ctl ls proc ; do PID != -1 -> Kill process; done Finally kill crinit """
    def crinit_kill_all_tasks(self):
        """ Kills all running crinit tasks and stops crinit """
        (stdout, stderr, ret) = self.__crinit_run("list", "Adding task failed", options="-f")

        if ret != 0:
            logger.error("Failed to list active crinit tasks.")
            return ret

        for line in io.StringIO(stdout):
            # Skip output header
            if line.startswith("NAME"):
                continue

            fields = line.split()

            # Skip stopped processes
            if fields[1] == -1:
                continue

            res = self.crinit_disable_task(fields[2])
            if res != 0:
                logger.error(f"Failed to disbale task {fields[2]}")
                return res

            res = self.crinit_kill_task(fields[2])
            if res != 0:
                logger.error(f"Failed to kill task {fields[2]}")
                return res

        return 0

    def crinit_restart_task(self, task_name):
        """Resets the status bits of <task_name> if it is DONE or FAILED.

        | task_name | The task name |
        """
        return self.__crinit_run("restart", "Restarting task failed", task=task_name)[2]

    def crinit_check_task_state(self, task_name, state):
        """Queries status bits and PID of <task_name>.

        | task_name | The task name |
        """
        stdout, stderr, ret = self.__crinit_run("status", "Requesting task status failed", task=task_name)
        if ret == 0:
            match = re.search(r"(?i)\bstatus\b\s*:\s*(?P<state>\w+)\s*,\s*\bpid\b\s*:\s*(?P<pid>[\+-]?\d+)", stdout)
            if match is not None:
                task_state = match.groupdict()

                return 0 if task_state['state'] == state else -1
            else:
                logger.error(f"Failed to parse crinit state for task {task_name}.")
                return -1

        return ret

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
