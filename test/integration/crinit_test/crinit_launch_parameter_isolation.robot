# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if parameters for the target
...               command are given correctly to crinit-launch

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
${TASK}        echo_test_service
${TASK_CONF}    SEPARATOR=\n
...             # Task to test elos events emitted by a succeeding task.
...             NAME = echo_test_service
...             COMMAND = /bin/timeout --preserve-status -s 15 -v 5 echo -ne "Hello, World!\\n"
...             USER = @@USER@@
...             GROUP = @@GROUP@@


*** Test Cases ***
Crinit Executes Task With Additional Parameters
    [Documentation]     Test if additional parameters are ignored by crinit-launch

    Given A Crinit Task With '${USER}' And '${GROUP}'
    When Crinit Starts The Task
    Then The Task Finished Successfully


*** Keywords ***
A Crinit Task With '${USER}' And '${GROUP}'
    [Documentation]    Set up a crinit task with given user and group

    ${TEMPORARY_CONF}=     Replace String     ${TASK_CONF}    @@USER@@    ${USER}
    ${TEMPORARY_CONF}=     Replace String     ${TEMPORARY_CONF}    @@GROUP@@    ${GROUP}

    Should Contain    ${TEMPORARY_CONF}    ${USER}
    Should Contain    ${TEMPORARY_CONF}    ${GROUP}

    Set Test Variable   ${NEW_CONF}    ${TEMPORARY_CONF}

Crinit Starts The Task
    [Documentation]   Start the crinit task with task config.

    Log     ${TASK}
    Log     ${NEW_CONF} 
    ${rc} =  Crinit Add Task Config    ${TASK}    ${NEW_CONF}
   
    Should Be Equal As Numbers  ${rc}  0
    ${rc} =  Crinit Add Task    ${TASK}.crinit
    Should Be Equal As Numbers  ${rc}  0

The Task Finished Successfully
    [Documentation]   Task must be finished successfully

    ${rc} =  Crinit Check Task State    ${TASK}    done
    Should Be Equal As Numbers  ${rc}  0
