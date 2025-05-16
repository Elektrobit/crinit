# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if signature verification is working correctly.

Resource          ../keywords.resource
Resource          ../elosd-keywords.resource
Resource          ../crinit-keywords.resource

Library           String
Library           SSHLibrary

Suite Setup       Prepare Target System
Suite Teardown    Clean Up Target System

Test Setup        Crinit Start    series_file=${SIG_TASK_DIR}/sigtest.series
Test Teardown     Crinit Stop

*** Variables ***
${SIG_TASK_DIR}             /etc/crinit/itest/sigtest
${LOCAL_TEST_DIR}           /tmp/crinit-sigtest
${PUBKEY_PATH}              /etc/crinit/itest/data/pubkeys/crinit-root-test-pub.der
${FAKE_CMDLINE_PATH}        /tmp/fake_cmdline
${FAKE_CMDLINE_PREPEND}     crinit.signatures=yes crinit.sigkeydir=/etc/crinit/itest/data/pubkeys

*** Test Cases ***
Crinit Tasks With Valid Signatures Get Loaded
    [Documentation]    Crinit loads tasks through a series file or crinit-ctl if their signatures are valid.
    [Setup]    Set Up A Test Case With Valid Task Configs
    Wait Until Keyword Succeeds  5s  200ms
    ...  Check If Tasks Have State    sigtask1  sigtask2  sigtask3    State=done

    Given Sigtask4 Is Set As Valid
    ${rc}    Then Crinit Add Task    sigtask4.crinit
    Should Be Equal As Numbers    ${rc}    0

    Wait Until Keyword Succeeds  5s  200ms
    ...  Check If Tasks Have State    sigtask1  sigtask2  sigtask3  sigtask4    State=done


Crinit Tasks With Invalid Signatures Cannot Be Loaded
    [Documentation]    Crinit will not load tasks through a series file or crinit-ctl if their signatures are not valid.
    [Setup]    Set Up A Test Case With Valid Task Configs
    Wait Until Keyword Succeeds  5s  200ms
    ...  Check If Tasks Have State    sigtask1  sigtask2  sigtask3    State=done

    Given Sigtask4 Is Set As Corrupted
    ${rc}    Then Crinit Add Task    sigtask4.crinit
    Should Not Be Equal As Numbers    ${rc}    0

*** Keywords ***
A Symbolic Link Has Been Created With
    [Arguments]    ${Name}=    ${PointingTo}=
    ${rc}  Execute And Log Based On User Permissions  sh -c "rm -f ${Name} && ln -s ${PointingTo} ${Name}"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0

Prepare Target System
    Connect To Target And Log In
    Enroll Crinit Root Key
    Mount Fake Kernel Cmdline To Enable Crinit Signatures
    Prepare Temporary Test Directory

Clean Up Target System
    ${rc}  Execute And Log Based On User Permissions  sh -c "rm -f ${LOCAL_TEST_DIR}/sigtask4.crinit"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    Unmount Fake Kernel Cmdline
    Unlink Crinit Root Key
    Close All Connections

Set Up A Test Case With Valid Task Configs
    Given Sigtask4 Is Set As Valid
    Then Crinit Start    series_file=${SIG_TASK_DIR}/sigtest.series

Sigtask4 Is Set As Valid
    A Symbolic Link Has Been Created With
        ...       Name=${LOCAL_TEST_DIR}/sigtask4.crinit  PointingTo=${LOCAL_TEST_DIR}/sigtask4.crinit.valid

Sigtask4 Is Set As Corrupted
    A Symbolic Link Has Been Created With
        ...       Name=${LOCAL_TEST_DIR}/sigtask4.crinit  PointingTo=${LOCAL_TEST_DIR}/sigtask4.crinit.modified

Prepare Temporary Test Directory
    ${rc}  Execute And Log Based On User Permissions
    ...  sh -c "mkdir -p ${LOCAL_TEST_DIR} && cp ${SIG_TASK_DIR}/sigtask4* ${LOCAL_TEST_DIR}"    ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0

Enroll Crinit Root Key
    ${key_id}    Execute And Log Based On User Permissions
    ...          keyctl session - enroll-itest-root-key.sh ${PUBKEY_PATH}    ${RETURN_STDOUT}
    Should Not Be Empty    ${key_id}
    Set Suite Variable    $CRINIT_ROOT_PK_ID    ${key_id}

Unlink Crinit Root Key
    ${rc}    Execute And Log Based On User Permissions    keyctl unlink ${CRINIT_ROOT_PK_ID}    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0

Mount Fake Kernel Cmdline To Enable Crinit Signatures
    ${rc}    Execute And Log    echo -n "${FAKE_CMDLINE_PREPEND} " > ${FAKE_CMDLINE_PATH}    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}    Execute And Log    cat /proc/cmdline >> ${FAKE_CMDLINE_PATH}    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}    Execute And Log Based On User Permissions
    ...      mount --bind ${FAKE_CMDLINE_PATH} /proc/cmdline    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}    Execute And Log    cat /proc/cmdline    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0

Unmount Fake Kernel Cmdline
    ${rc}    Execute And Log Based On User Permissions   umount /proc/cmdline    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}    Execute And Log    cat /proc/cmdline    ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
