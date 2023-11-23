# SPDX-License-Identifier: MIT
TARGET_IP = "172.17.0.2"
TARGET_USER = "target"
TARGET_PASSWORD = "target123"
TARGET_USER_IS_ROOT = "False"
CRINIT_SOCK =  "/tmp/crinit-itest.sock"
ELOS_STATUS_COMMAND = f"sh -c \"export CRINIT_SOCK={CRINIT_SOCK}; crinit-ctl status elosd\""
