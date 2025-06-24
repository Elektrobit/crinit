# SPDX-License-Identifier: MIT
*** Settings ***
Documentation        A test suite to check if RESPAWN behavies correctly after stop command received

Resource             ../keywords.resource

Library              ../libraries/CrinitLibrary.py

Suite Setup          Connect To Target And Log In
Suite Teardown       Close All Connections

*** Variables ***
${SERIES_DIR}        /etc/crinit/itest
${LOCAL_TEST_DIR}    /tmp

${TASK_RESPAWN}       respawning_task
${TASK_OUT_SINGLE}   ${LOCAL_TEST_DIR}/${TASK_RESPAWN}.out
${TASK_CONF_RESPAWN}  SEPARATOR=\n
...    # Test task with RESPAWN configured
...    NAME = ${TASK_RESPAWN}
...    COMMAND = /bin/cat
...    RESPAWN = YES
...    RESPAWN_RETRIES = 5

*** Test Cases ***
Crinit Stops Task And Doesnot Respawn It
    [Documentation]    "Crinit stops a task after receiving stop command and ensures it stays stopped despite respawn"
    [Setup]    Crinit Start
    Given The Crinit Task ${TASK_RESPAWN} Containing ${TASK_CONF_RESPAWN} Is Loaded
        And Ensure The Task ${TASK_RESPAWN} Is Running
    When Crinit Stop Task    ${TASK_RESPAWN}
        Then Ensure The Task ${TASK_RESPAWN} Is Not Running
    [Teardown]    Crinit Stop
