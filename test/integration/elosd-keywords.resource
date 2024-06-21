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
        ${publish_output}=    Execute Command    elosc -P 54322 -p '${message}'
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

Check That An Elos Event From Crinit Was Logged With
    [Arguments]        ${severity}=    ${classification}=    ${msgcode}=    ${payload}=
    [Documentation]    Check if at least one event from Crinit, filtered by severity, classification, message code, and
    ...                payload was logged.

    ${elosc_out}=    Execute Command
    ...    elosc -P 54322 -f \".event.source.appName \'crinit\' STRCMP .event.severity ${severity} EQ AND .event.payload \'${payload}\' STRCMP AND .event.messageCode ${msgcode} EQ AND .event.classification ${classification} EQ AND\"
    Should Match Regexp    ${elosc_out}    \{.+\}
