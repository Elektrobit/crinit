# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if crinit can execute a task with
...               an user and group that are not privileged.

Resource          ../crinit-keywords.resource

Library           String
Library           SSHLibrary
Library           DataDriver    file=test_data/test_capability_config.csv    dialect=unix

Suite Setup       Connect To Target And Log In
Suite Teardown    Close All Connections

Test Teardown     Crinit Stop

Test Template    Crinit Creates Task With Capability Configuration


*** Variables ***
${SERIES_DIR}             /etc/crinit/itest
${LOCAL_TEST_DIR}           /tmp
${TASK}        test_service
${TASK_CONFIG}  SEPARATOR=\n
...             NAME = test_service
...             COMMAND = /bin/sleep 10
...             USER = nobody
...             GROUP = nogroup
...             @@CAP_SET_KEY@@ = @@CAP_SET_VAL@@
...             @@CAP_CLEAR_KEY@@ = @@CAP_CLEAR_VAL@@

*** Test Cases ***
Crinit Creates Task With Capability Configuration ${cap_set_key} ${cap_set_val} ${cap_clear_key} ${cap_clear_val} ${exp_cap_proc} ${exp_task_creation}    Default    UserData

*** Keywords ***
Crinit Creates Task With Capability Configuration
    [Documentation]     Test if capability configuration is obeyed
    [Arguments]         ${cap_default_key}    ${cap_default_val}    ${cap_set_key}    ${cap_set_val}    ${cap_clear_key}    ${cap_clear_val}    ${exp_cap_proc}    ${exp_task_creation}
    
    Given Crint Start With Default Capabilities ${cap_default_val}
    
    And A Task Config ${TASK_CONFIG}

    When Crinit Add Task Config    ${TASK}    ${TASK_CONFIG}

    And Crinit Add Task    ${TASK}.crinit
    
    Then Crinit Task Creation Was '${exp_task_creation}'

    And The '${TASK}' Should Have The Capabilities '${exp_cap_proc}'
