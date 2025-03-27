# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if crinit can execute a task with
...               an user and group that are not privileged.

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
${SERIES_DIR}             /etc/crinit/itest
${LOCAL_TEST_DIR}           /tmp
${USER}        nobody
${GROUP}       nogroup
${MULTIGROUPS}      nogroup disk floppy
${TASK}        test_service
${TASK_CONF}    SEPARATOR=\n
...             # Task to test elos events emitted by a succeeding task.
...             NAME = test_service
...             COMMAND = /bin/sleep 60
...             USER = @@USER@@
...             GROUP = @@GROUP@@


*** Test Cases ***
Crinit Executes Task As Different User And Group
    [Documentation]     Test if user and group configuration is obeyed

    Given A Crinit Task With '${USER}' And '${GROUP}'
    When Crinit Starts The Task
    Then Wait Until Keyword Succeeds  5s  200ms  The Task Is Owned By '${USER}' And '${GROUP}'


Crinit Executes Task With Supplementary Groups
    [Documentation]     Test if supplementary groups are set correctly

    Given A Crinit Task With '${USER}' And '${MULTIGROUPS}'
    When Crinit Starts The Task
    Then Wait Until Keyword Succeeds  5s  200ms  The Task Has Owner '${USER}' And Supplementary Groups '${MULTIGROUPS}'
    

*** Keywords ***
A Crinit Task With '${USER}' And '${GROUP}'
    [Documentation]    Set up a crinit task with given user and group

    ${TEMPORARY_CONF}=     Replace String     ${TASK_CONF}    @@USER@@    ${USER}
    ${TEMPORARY_CONF}=     Replace String     ${TEMPORARY_CONF}    @@GROUP@@    ${GROUP}

    Should Contain    ${TEMPORARY_CONF}    ${USER}
    Should Contain    ${TEMPORARY_CONF}    ${GROUP}

    Set Test Variable   ${NEW_CONF}    ${TEMPORARY_CONF}


Check If '${Task}' Is Running
    [Documentation]     Pass return value from Crinit Check Task State to caller

    ${rc} =  Crinit Check Task State    ${TASK}    running
    Should Be Equal As Numbers  ${rc}  0


Crinit Starts The Task
    [Documentation]   Start the crinit task with task config.

    Log     ${TASK}
    Log     ${NEW_CONF} 
    ${rc} =  Crinit Add Task Config    ${TASK}    ${NEW_CONF}
   
    Should Be Equal As Numbers  ${rc}  0
    ${rc} =  Crinit Add Task    ${TASK}.crinit
    Should Be Equal As Numbers  ${rc}  0
    Wait Until Keyword Succeeds  5s  200ms  Check If '${Task}' Is Running 


The Task Is Owned By '${USER}' And '${GROUP}'
    [Documentation]    Check if started crinit task is owned by
    ...                given user and group

    ${rc} =  '${TASK}' User Is '${USER}' And Group Is '${GROUP}'
    Should Be Equal As Numbers  ${rc}  0


The Task Has Owner '${USER}' And Supplementary Groups '${MULTIGROUPS}'
    [Documentation]    Check if started crinit task is owned by
    ...                given user and belongs to the given groups

    ${rc} =  '${TASK}' User Is '${USER}' And Supplementary Groups Are '${MULTIGROUPS}'
    Should Be Equal As Numbers  ${rc}  0
