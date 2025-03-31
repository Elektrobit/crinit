# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if Crinit emits the correct task events to elos during lifetime of a task.

Resource          ../keywords.resource
Resource          ../elosd-keywords.resource
Resource          ../crinit-keywords.resource

Library           String
Library           SSHLibrary

Suite Setup       Connect To Target And Log In
Suite Teardown    Close All Connections

Test Setup        Crinit Start
Test Teardown     Crinit Stop

*** Variables ***
${LOCAL_TEST_DIR}           /tmp
${SUCCESS_TASK_NAME}    success_service
${SUCCESS_TASK_CONF}    SEPARATOR=\n
...                         # Task to test elos events emitted by a succeeding task.
...                         NAME = success_service
...                         COMMAND = /bin/true
${FAILURE_TASK_NAME}    fail_service
${FAILURE_TASK_CONF}    SEPARATOR=\n
...                         # Task to test elos events emitted by a failing task.
...                         NAME = fail_service
...                         COMMAND = /bin/false

*** Test Cases ***
Crinit Sends All Relevant Events Of A Successful Task execution To Elos
    [Documentation]    Checks if Crinit sends all relvant events to elos in case of a successful task.

    Given Elosd Has Been Started
        And The Crinit Task ${SUCCESS_TASK_NAME} Containing ${SUCCESS_TASK_CONF} Is Loaded
    When Wait Until Keyword Succeeds  5s  200ms   The Task ${SUCCESS_TASK_NAME} Has Exited Successfully

    # Task creation event
    # msgcode: FILE_OPENED, severity: INFO, classification: PROCESS
    Then Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
         ...    severity=4  msgcode=2003  classification=32  payload=${SUCCESS_TASK_NAME}
    # Task start event
    # msgcode: PROCESS_CREATED, severity: INFO, classification: PROCESS
    And Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
        ...    severity=4  msgcode=2001  classification=32  payload=${SUCCESS_TASK_NAME}
    # Successful task end event
    # msgcode: PROCESS_EXITED, severity: INFO, classification: PROCESS
    And Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
        ...    severity=4  msgcode=2002  classification=32  payload=${SUCCESS_TASK_NAME}

Crinit Sends All Relevant Events Of An Unsuccessful Task execution To Elos
    [Documentation]    Checks if Crinit sends all relvant events to elos in case of an unsuccessful task.
    Given Elosd Has Been Started
        And The Crinit Task ${FAILURE_TASK_NAME} Containing ${FAILURE_TASK_CONF} Is Loaded
    When Wait Until Keyword Succeeds  5s  200ms   The Task ${FAILURE_TASK_NAME} Has Exited Unsuccessfully

    # Task creation event
    # msgcode: FILE_OPENED, severity: INFO, classification: PROCESS
    Then Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
         ...    severity=4  msgcode=2003  classification=32  payload=${FAILURE_TASK_NAME}
    # Task start event
    # msgcode: PROCESS_CREATED, severity: INFO, classification: PROCESS
    And Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
        ...    severity=4  msgcode=2001  classification=32  payload=${FAILURE_TASK_NAME}
    # Unsuccessful task end event
    # msgcode: EXIT_FAILURE, severity: ERROR, classification: (PROCESS | PROCESS_ERRORS)
    And Wait Until Keyword Succeeds  5s  200ms    Check That An Elos Event From Crinit Was Logged With
        ...    severity=2  msgcode=5006  classification=544  payload=${FAILURE_TASK_NAME}

*** Keywords ***
Elosd Has Been Started
    ${running} =  Elosd Is Running
    Should Be True  ${running}
