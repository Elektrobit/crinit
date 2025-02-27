# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if the --child-subreaper CLI option behaves correctly.

Resource          ../keywords.resource
Resource          ../elosd-keywords.resource
Resource          ../crinit-keywords.resource

Library           String
Library           SSHLibrary

Suite Setup       Connect To Target
Suite Teardown    Disconnect From Target

*** Test Cases ***
Crinit Sets Subreaper Process Attribute To True If Option Is Activated
    [Documentation]    "Crinit uses prctl() with the right arguments for --child-subreaper."
    [Setup]    Create Temporary Directory For Test Case
    Crinit Start  strace_output=${TEST_TEMPDIR}/crinit.trace  strace_filter=prctl  crinit_args=--child-subreaper
    Crinit Stop  crinit_args=--child-subreaper 
    ${rc}  Execute And Log Based On User Permissions
    ...   grep -q "^prctl(PR_SET_CHILD_SUBREAPER, 1)" "${TEST_TEMPDIR}/crinit.trace"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    [Teardown]    Remove Temporary Directory For Test Case

Crinit Sets Subreaper Process Attribute To False If Option Is Deactivated
    [Documentation]    "Crinit uses prctl() with the right arguments for --no-child-subreaper."
    [Setup]    Create Temporary Directory For Test Case
    Crinit Start  strace_output=${TEST_TEMPDIR}/crinit.trace  strace_filter=prctl  crinit_args=--no-child-subreaper
    Crinit Stop  crinit_args=--no-child-subreaper 
    ${rc}  Execute And Log Based On User Permissions
    ...   grep -q "^prctl(PR_SET_CHILD_SUBREAPER, 0)" "${TEST_TEMPDIR}/crinit.trace"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    [Teardown]    Remove Temporary Directory For Test Case

Crinit Does Not Set Subreaper Process Attribute If Option Is Left Unset
    [Documentation]    "Crinit does not use PR_SET_CHILD_SUBREAPER if no option is given."
    [Setup]    Create Temporary Directory For Test Case
    Crinit Start  strace_output=${TEST_TEMPDIR}/crinit.trace  strace_filter=prctl
    Crinit Stop
    ${rc}  Execute And Log Based On User Permissions
    ...   grep -q "PR_SET_CHILD_SUBREAPER" "${TEST_TEMPDIR}/crinit.trace"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    1
    [Teardown]    Remove Temporary Directory For Test Case

*** Keywords ***
Connect To Target
    Connect To Target And Log In

Disconnect From Target
    Close All Connections

Create Temporary Directory For Test Case
    ${stdout}  Execute And Log Based On User Permissions  mktemp -d  ${RETURN_STDOUT}
    Should Not Be Empty    ${stdout}
    Set Test Variable    ${TEST_TEMPDIR}    ${stdout}

Remove Temporary Directory For Test Case
    Should Not Be Empty    ${TEST_TEMPDIR}
    ${rc}  Execute And Log Based On User Permissions  rm -rf "${TEST_TEMPDIR}"
