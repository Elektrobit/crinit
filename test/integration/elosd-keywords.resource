# SPDX-License-Identifier: MIT
*** Settings ***
Documentation    Collection of high level keywords for direct interaction with elosd

Resource         keywords.resource

*** Keywords ***
Elos Publish Event List
    [Arguments]    @{message_list}
    [Documentation]    Publish a given list of messages

    @{publish_log}=    Create List
    FOR    ${message}    IN    @{message_list}
        ${publish_output}=    Execute Command    elosc -p '${message}'
        Append To List    ${publish_log}    ${publish_output}
    END

    Log List    ${publish_log}

Elosd Is Running
    [Documentation]    Elosd is still running

    ${is_running}=    Set Variable    ${false}
    ${running}=    Set Variable    Running

    ${status}    ${rc}    Execute And Log Based On User Permissions    ${ELOS_STATUS_COMMAND}    ${RETURN_STDOUT}    ${RETURN_RC}
    ${is_running}=    Run Keyword And Return Status    Should Contain    ${status}    ${running}    ignore_case=True

    RETURN    ${is_running}
