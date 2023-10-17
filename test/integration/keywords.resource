# SPDX-License-Identifier: MIT
*** Settings ***
Documentation    Collection of high level keywords shared by the test cases
Library          String
Library          SSHLibrary
Library          Collections

*** Variables ***
${TARGET}          ${TARGET_IP}
${SSH_PORT}        22
${USERNAME}        ${TARGET_USER}
${PASSWORD}        ${TARGET_PASSWORD}
${RETRY_TIME}      2m
${TIMEOUT}         20s
${USER_IS_ROOT}    ${TARGET_USER_IS_ROOT}

# Variables for Execute And Log
${RETURN_RC}       rc
${RETURN_STDOUT}   stdout
${RETURN_STDERR}   stderr

*** Keywords ***
Connect To Target And Log In
    [Documentation]    Opens a new SSH connection to the target system.
    ...
    ...    It uses the following global variables for establishing the target connection:
    ...
    ...    | \${TARGET}     | The host name or IP address of the target system |
    ...    | \${USERNAME} | The remote user name, e.g. ``root`` |
    ...    | \${PASSWORD} | The remote user's password |
    ...
    ...    | \${LINUX_VERSION} | Used for storing the used Linux version. |

    Open Connection    ${TARGET}    timeout=${TIMEOUT}    port=${SSH_PORT}
    Wait Until Keyword Succeeds  ${RETRY_TIME}  ${TIMEOUT}
    ...   Login  ${USERNAME}  ${PASSWORD}

Execute And Log
    [Arguments]         ${command}    @{options}
    [Documentation]     Runs Execute Command on remote, logs stdout, stderr and return code and returns the specified options to the caller

    ${stdout}    ${stderr}    ${rc}    Execute Command    ${command}      return_stdout=true    return_stderr=true    return_rc=true

    Log   ${stderr}
    Log   ${stdout}
    Log   ${rc}

    @{return_list}=    Create List

    FOR     ${option}   IN   @{options}
        Run Keyword If    "${option}"=="${RETURN_STDOUT}"    Append To List    ${return_list}    ${stdout}
        Run Keyword If    "${option}"=="${RETURN_STDERR}"    Append To List    ${return_list}    ${stderr}
        Run Keyword If    "${option}"=="${RETURN_RC}"        Append To List    ${return_list}    ${rc}
    END

    ${return_list_length}=           Get Length   ${return_list}
    Return From Keyword If    ${return_list_length}==1    ${return_list}[0]
    Return From Keyword If    ${return_list_length}>1     @{return_list}

Execute And Log Based On User Permissions
    [Arguments]         ${command}    @{options}
    [Documentation]     Runs Execute Command on remote based on user permission, logs stdout, stderr and return code and returns the specified options to the caller

    IF    '${USER_IS_ROOT}' == 'True'
        ${stdout}    ${stderr}    ${rc}    Execute Command    ${command}      return_stdout=true    return_stderr=true    return_rc=true
    ELSE
        ${stdout}    ${stderr}    ${rc}    Execute Command    ${command}      sudo=True    sudo_password=${PASSWORD}    return_stdout=true    return_stderr=true    return_rc=true
    END

    Log    ${stderr}
    Log    ${stdout}
    Log    ${rc}

    @{return_list}=    Create List

    FOR     ${option}   IN   @{options}
        Run Keyword If    "${option}"=="${RETURN_STDOUT}"    Append To List    ${return_list}    ${stdout}
        Run Keyword If    "${option}"=="${RETURN_STDERR}"    Append To List    ${return_list}    ${stderr}
        Run Keyword If    "${option}"=="${RETURN_RC}"        Append To List    ${return_list}    ${rc}
    END

    ${return_list_length}=           Get Length   ${return_list}
    Return From Keyword If    ${return_list_length}==1    ${return_list}[0]
    Return From Keyword If    ${return_list_length}>1     @{return_list}
