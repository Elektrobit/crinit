# SPDX-License-Identifier: MIT
*** Settings ***
Documentation        A test suite to check if STOP_COMMAND behaves as expected.

Resource             ../keywords.resource
Resource             ../elosd-keywords.resource
Resource             ../crinit-keywords.resource

Library               String
Library              SSHLibrary

Suite Setup          Connect To Target
Suite Teardown       Disconnect From Target

*** Variables ***
${SERIES_DIR}        /etc/crinit/itest
${LOCAL_TEST_DIR}    /tmp

${TASK_SINGLE}       single_stopcmd_task
${TASK_OUT_SINGLE}   ${LOCAL_TEST_DIR}/${TASK_SINGLE}.out
${TASK_CONF_SINGLE}  SEPARATOR=\n
...    # Test task with a single STOP_COMMAND
...    NAME = ${TASK_SINGLE}
...    COMMAND = /bin/sleep 3600s
...    STOP_COMMAND = /bin/touch ${TASK_OUT_SINGLE}

${TASK_MULTI}              multi_stopcmd_task
${TASK_OUT_MULTI_FIRST}    ${LOCAL_TEST_DIR}/${TASK_MULTI}-1.out
${TASK_OUT_MULTI_SECOND}   ${LOCAL_TEST_DIR}/${TASK_MULTI}-2.out
${TASK_OUT_MULTI_THIRD}    ${LOCAL_TEST_DIR}/${TASK_MULTI}-3.out
${TASK_CONF_MULTI}         SEPARATOR=\n
...    # Test task with multiple STOP_COMMANDs
...    NAME = ${TASK_MULTI}
...    COMMAND = /bin/sleep 3600s
...    STOP_COMMAND = /bin/touch ${TASK_OUT_MULTI_FIRST}
...    STOP_COMMAND = /bin/touch ${TASK_OUT_MULTI_SECOND}
...    STOP_COMMAND = /bin/touch ${TASK_OUT_MULTI_THIRD}

*** Test Cases ***
Crinit Runs Singular Stop Command If Requested
    [Documentation]    "Crinit runs configured STOP_COMMAND for single_stopcmd_task"
    [Setup]    Clean Up Test Artifacts
    Crinit Start
    Given The Crinit Task ${TASK_SINGLE} Containing ${TASK_CONF_SINGLE} Is Loaded
    When Wait Until Keyword Succeeds  5s  200ms    The Task ${TASK_SINGLE} Is Currently Running
    Then Crinit Stop Task    ${TASK_SINGLE}
      And Wait Until Keyword Succeeds  5s  200ms
          ...    Ensure Regular Files Exist    ${TASK_OUT_SINGLE}
    [Teardown]    Crinit Stop

Crinit Runs Multiple Stop Commands If Requested
    [Documentation]    "Crinit runs configured STOP_COMMANDs for multi_stopcmd_task"
    [Setup]    Clean Up Test Artifacts
    Crinit Start
    Given The Crinit Task ${TASK_MULTI} Containing ${TASK_CONF_MULTI} Is Loaded
    When Wait Until Keyword Succeeds  5s  200ms    The Task ${TASK_MULTI} Is Currently Running
    Then Crinit Stop Task    ${TASK_MULTI}
      And Wait Until Keyword Succeeds  5s  200ms
          ...    Ensure Regular Files Exist
          ...      ${TASK_OUT_MULTI_FIRST}    ${TASK_OUT_MULTI_SECOND}    ${TASK_OUT_MULTI_THIRD}
    [Teardown]    Crinit Stop

Crinit Runs All Stop Commands On Shutdown
    [Documentation]    "Crinit runs all configured STOP_COMMANDs when a shutdown is performed."
    [Setup]    Clean Up Test Artifacts
    Crinit Start    ld_preload=libitest-stop-cmd-overrides.so
    Given The Crinit Task ${TASK_SINGLE} Containing ${TASK_CONF_SINGLE} Is Loaded
      And The Crinit Task ${TASK_MULTI} Containing ${TASK_CONF_MULTI} Is Loaded
    When Wait Until Keyword Succeeds  5s  200ms    The Task ${TASK_SINGLE} Is Currently Running
      And Wait Until Keyword Succeeds  5s  200ms    The Task ${TASK_MULTI} Is Currently Running
    Then Crinit Poweroff
      And Wait Until Keyword Succeeds  10s  200ms
          ...    Ensure Regular Files Exist
          ...      ${TASK_OUT_SINGLE}
          ...      ${TASK_OUT_MULTI_FIRST}    ${TASK_OUT_MULTI_SECOND}    ${TASK_OUT_MULTI_THIRD}
    [Teardown]    Crinit Stop

*** Keywords ***
Connect To Target
    Connect To Target And Log In

Disconnect From Target
    Close All Connections

Clean Up Test Artifacts
    ${rc}  Execute And Log Based On User Permissions
    ...    rm -f ${TASK_OUT_SINGLE} ${TASK_OUT_MULTI_FIRST} ${TASK_OUT_MULTI_SECOND} ${TASK_OUT_MULTI_THIRD}
    ...    ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0

Ensure Regular Files Exist
    [Arguments]    @{files}
    FOR    ${file}    IN    @{files}
        ${rc}  Execute And Log Based On User Permissions
        ...    test -f ${file}
        ...    ${RETURN_RC}
        Should Be Equal As Numbers    ${rc}    0
    END
