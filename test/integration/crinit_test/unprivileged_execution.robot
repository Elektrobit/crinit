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
${TASK}        test_service
${TASK_CONF}    SEPARATOR=\n
...             # Task to test elos events emitted by a succeeding task.
...             NAME = test_service
...             COMMAND = /usr/bin/sleep 60
...             USER = user
...             GROUP = grp


*** Test Cases ***
Crinit Executes Task As Different User And Group
    [Documentation]     Test if user and group configuration is obeyed

    Given A Crinit Task With '${USER}' And '${GROUP}'
    When Crinit Starts The Task
    Then The Task Is Owned By '${USER}' And '${GROUP}'


*** Keywords ***
A Crinit Task With '${USER}' And '${GROUP}'
    [Documentation]    Set up a crinit task with given user and group

    ${task_conf}=     Replace String     ${TASK_CONF}    user    ${USER} 
    ${task_conf}=     Replace String     ${task_conf}    grp    ${GROUP}

    Should Contain    ${task_conf}    ${USER}
    Should Contain    ${task_conf}    ${GROUP}

    Set Test Variable   ${NEW_CONF}    ${task_conf} 

Crinit Starts The Task
    [Documentation]   Start the crinit task with task config.

    Log     ${TASK}
    Log     ${NEW_CONF} 
    ${rc} =  Crinit Add Task Config    ${TASK}    ${NEW_CONF}
   
    Should Be Equal As Numbers  ${rc}  0
    ${rc} =  Crinit Add Task    ${TASK}.crinit
    Should Be Equal As Numbers  ${rc}  0
    ${rc} =  Crinit Check Task State    ${TASK}    running
    Should Be Equal As Numbers  ${rc}  0


The Task Is Owned By '${USER}' And '${GROUP}'
    [Documentation]    Check if started crinit task is owned by
    ...                given user and group

    ${rc} =  '${TASK}' User Is '${USER}' And Group Is '${GROUP}'
    Should Be Equal As Numbers  ${rc}  0
