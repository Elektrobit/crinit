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
${ELOS_DEPEND_TASK_NAME}    test_service
${ELOS_DEPEND_TASK_CONF}    SEPARATOR=\n
...                         # Loads a test service with an elos dependency
...
...                         NAME = test_service
...
...                         COMMAND = /bin/true
...
...                         FILTER_DEFINE = TEST_FILTER ".event.source.appName 'popocatepetl' STRCMP"
...                         DEPENDS = @elos:TEST_FILTER
...
...                         RESPAWN = NO
@{MESSAGES}                 {"source": {"appName": "popocatepetl"}}

*** Test Cases ***
Crinit Tasks With Event Filter Dependencies Get Started
    [Documentation]    Crinit starts a task with an event filter dependency
    ...                if the correct event is published.

    Given Elosd Is Running
        AND A Crinit Task Depending On An Elos Event Filter Is Started
    When Elos Publishes The Dependency Event
    Then Crinit Starts The Depending Task

*** Keywords ***
A Crinit Task Depending On An Elos Event Filter Is Started
    Crinit Add Task Config    ${ELOS_DEPEND_TASK_NAME}    ${ELOS_DEPEND_TASK_CONF}
    Crinit Add Task    ${ELOS_DEPEND_TASK_NAME}.crinit

Elos Publishes The Dependency Event
    Elos Publish Event List    @{MESSAGES}

Crinit Starts The Depending Task
    Crinit Check Task State    ${ELOS_DEPEND_TASK_NAME}    done
