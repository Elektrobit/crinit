# SPDX-License-Identifier: MIT
*** Settings ***
Documentation    Collection of high level keywords for direct interaction with crinit

Resource         keywords.resource

Library          libraries/CrinitLibrary.py

*** Variables ***
@{TASK_DIRECTORY}            ./service/crinit/

*** Keywords ***
Crinit Load Test Tasks
    [Documentation]    Loads all crinit test tasks
    ${files}=    List Files In Directory    ${TASK_DIRECTORY}

    FOR    ${file}    IN    @{files}
        Crinit Add Task    ${file}
    END

Check If Tasks Have State
    [Arguments]    @{TASKS}    ${State}=
    FOR    ${Task}    IN    @{TASKS}
        ${rc} =  Crinit Check Task State    ${Task}    ${State}
        Should Be Equal As Numbers  ${rc}  0
    END
