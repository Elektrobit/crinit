# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if an elos event depending crinit task
...               is started if the defined filter is triggered.

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
${START_TASK_NAME}    test_starter
${START_TASK_CONF}    SEPARATOR=\n
...                   # Loads a test service with some dependency
...                   NAME = ${START_TASK_NAME}
...                   COMMAND = /bin/true
${DEPEND_TASK_NAME}    test_depends
${DEPEND_TASK_CONF}    SEPARATOR=\n
...                    # Loads a test service with some dependency
...                    NAME = ${DEPEND_TASK_NAME}
...                    COMMAND = /bin/true
...                    DEPENDS = never_run:wait ${START_TASK_NAME}:wait
${TRIGGER_TASK_NAME}    test_trigger
${TRIGGER_TASK_CONF}    SEPARATOR=\n
...                     # Loads a test service with some trigger
...                     NAME = ${TRIGGER_TASK_NAME}
...                     COMMAND = /bin/true
...                     TRIGGER = never_run:wait ${START_TASK_NAME}:wait
${TRIGGER_ELOS_TASK_NAME}    test_elos_trigger
${TRIGGER_ELOS_TASK_CONF}    SEPARATOR=\n
...                          # Loads a test service with a elos filter trigger
...                          NAME = ${TRIGGER_ELOS_TASK_NAME}
...                          COMMAND = /bin/true
...                          FILTER_DEFINE = TEST_FILTER ".event.source.appName 'popocatepetl' STRCMP"
...                          TRIGGER = never_run:wait @elos:TEST_FILTER
@{ELOS_MESSAGES}             {"source": {"appName": "popocatepetl"}}
${TRIGGER_REARM_TASK_NAME}   test_trigger_rearm
${TRIGGER_REARM_OUT_FILE}    /tmp/trigger_rearm.log
${TRIGGER_REARM_TASK_CONF}   SEPARATOR=\n
...                          # Loads a test service with some rearming trigger
...                          NAME = ${TRIGGER_REARM_TASK_NAME}
...                          COMMAND = /bin/echo "successfully triggered"
...                          IO_REDIRECT = STDOUT "${TRIGGER_REARM_OUT_FILE}" APPEND
...                          TRIGGER = @ctl:enable
...                          TRIGGER_REARM=YES


*** Test Cases ***
Crinit Tasks With Trigger Get Started
    [Documentation]    Crinit starts a task with trigger

    Given Triggerd And Depending Tasks Are Loaded
    When Start Task Is Loaded
    Then Triggered Task Is Started
    And Depending Task Is Not Started

Crinit Trigger Task With Elos Event
    [Documentation]    Crinit starts a task triggered on an elos event

    Given Elosd Has Been Started
    And Elos Triggered Task Is Loaded
    When Elos Published Trigger Event
    Then Elos Triggered Task Is Started

Crinit Trigger Rearming Task Starts On Each Trigger
    [Documentation]    Crinit starts a task with rearming trigger on each trigger again

    Given Trigger Rearming Task Is Loaded
    And Trigger Rearming Task Is Triggered 3 Times
    Then Trigger Rearming Taks Is Loaded Again
    And Trigger Rearming Task Has Run 3 Times


*** Keywords ***
Task Is Loaded
    [Arguments]     ${task_name}    ${task_conf}
    ${rc} =  Crinit Add Task Config    ${task_name}    ${task_conf}
    Should Be Equal As Numbers  ${rc}  0
    ${rc} =  Crinit Add Task    ${task_name}.crinit
    Should Be Equal As Numbers  ${rc}  0

Trigger Rearming Task Is Loaded
    Task Is Loaded      ${TRIGGER_REARM_TASK_NAME}    ${TRIGGER_REARM_TASK_CONF}

Elos Triggered Task Is Loaded
    And Task Is Loaded      ${TRIGGER_ELOS_TASK_NAME}    ${TRIGGER_ELOS_TASK_CONF}

Start Task Is Loaded
    Task Is Loaded    ${START_TASK_NAME}      ${START_TASK_CONF}

Triggerd And Depending Tasks Are Loaded
    Task Is Loaded    ${TRIGGER_TASK_NAME}    ${TRIGGER_TASK_CONF}
    Task Is Loaded    ${DEPEND_TASK_NAME}     ${DEPEND_TASK_CONF}

Elos Published Trigger Event
    Elos Publish Event List    @{ELOS_MESSAGES}

Trigger Rearming Task Is Triggered 3 Times
    Crinit Enable Task   ${TRIGGER_REARM_TASK_NAME}
    Crinit Enable Task   ${TRIGGER_REARM_TASK_NAME}
    Crinit Enable Task   ${TRIGGER_REARM_TASK_NAME}

Trigger Rearming Task Has Run 3 Times
    ${stdout}   ${rc}    Execute And Log Based On User Permissions
    ...         wc -l ${TRIGGER_REARM_OUT_FILE}
    ...         ${RETURN_STDOUT}    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}   0
    Should Start With   ${stdout}   3

Check '${task_name}' Is '${task_state}'
    ${rc} =  Crinit Check Task State    ${task_name}    ${task_state}
    Should Be Equal As Numbers  ${rc}  0

Trigger Rearming Taks Is Loaded Again
    Wait Until Keyword Succeeds  5s  200ms  Check '${TRIGGER_REARM_TASK_NAME}' Is 'loaded'

Triggered Task Is Started
    Wait Until Keyword Succeeds  5s  200ms  Check '${TRIGGER_TASK_NAME}' Is 'done'

Elos Triggered Task Is Started
    Wait Until Keyword Succeeds  5s  200ms  Check '${TRIGGER_ELOS_TASK_NAME}' Is 'done'

Depending Task Is Not Started
    Wait Until Keyword Succeeds  5s  200ms  Check '${DEPEND_TASK_NAME}' Is 'loaded'

Elosd Has Been Started
    ${running} =  Elosd Is Running
    Should Be True  ${running}
