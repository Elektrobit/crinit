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

Test Setup        Crinit Start
Test Teardown     Crinit Stop

Test Template    Crinit Creates Task With Capability Configuration


*** Variables ***
${SERIES_DIR}             /etc/crinit/itest
${LOCAL_TEST_DIR}           /tmp
${USER}        nobody
${GROUP}       nogroup
${TASK}        test_service
${TASK_CONF}    SEPARATOR=\n
...             NAME = test_service
...             COMMAND = /bin/sleep 10
...             USER = @@USER@@
...             GROUP = @@GROUP@@
...             @@CAP_SET_KEY@@ = @@CAP_SET_VAL@@
...             @@CAP_CLEAR_KEY@@ = @@CAP_CLEAR_VAL@@

*** Test Cases ***
Crinit Creates Task With Capability Configuration ${cap_set_key} ${cap_set_val} ${cap_clear_key} ${cap_clear_val} ${exp_res}    Default    UserData

*** Keywords ***
Crinit Creates Task With Capability Configuration
    [Documentation]     Test if user and group configuration is obeyed
    [Arguments]         ${cap_set_key}    ${cap_set_val}    ${cap_clear_key}    ${cap_clear_val}    ${exp_res}

    Given A Task Config

    When Crinit Starts The Task With Config

    Then Crinit Task Creation Was '${exp_res}'
