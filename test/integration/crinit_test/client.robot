# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if the crinit-client library is installed and running on the target

Resource          ../keywords.resource

Library           SSHLibrary

Suite Setup       Connect To Target And Log In
Suite Teardown    Close All Connections

*** Variables ***

*** Test Cases ***
Check If Crinit Client Is Installed
    [Documentation]    Test checks if crinit-ctl is installed
    ${output}=         Is Crinit Client Installed
    Should Not Be Empty    ${output}    msg=Crinit client is not installed

Check If Crinit Client Library Is Installed
    [Documentation]        Test checks if the crinit-client library is installed
    ${output}              Is Crinit Client Library Installed
    Should Not Be Empty    ${output}    msg=Crinit client library is not installed

*** Keywords ***
Is Crinit Client Installed
    ${value}=         Execute And Log    which crinit-ctl    ${RETURN_STDOUT}
    RETURN    ${value}

Is Crinit Client Library Installed
    ${value}=         Execute And Log    find /lib/* /usr/lib/* /usr/local/lib/* -name "libcrinit-client.so*" 2>/dev/null    ${RETURN_STDOUT}
    RETURN    ${value}
