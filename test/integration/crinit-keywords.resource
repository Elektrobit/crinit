# SPDX-License-Identifier: MIT
*** Settings ***
Documentation    Collection of high level keywords for direct interaction with crinit

Resource         keywords.resource

Library          libraries/CrinitLibrary.py

*** Variables ***
@{TASK_DIRECTORY}            ./service/crinit/
${PUBKEY_PATH}               /etc/crinit/itest/data/pubkeys/crinit-root-test-pub.der
${FAKE_CMDLINE_PATH}         /tmp/fake_cmdline
${FAKE_CMDLINE_PREPEND}      crinit.signatures=yes crinit.sigkeydir=/etc/crinit/itest/data/pubkeys

*** Keywords ***
Crinit Load Test Tasks
    [Documentation]    Loads all crinit test tasks
    ${files}=    List Files In Directory    ${TASK_DIRECTORY}

    FOR    ${file}    IN    @{files}
        Crinit Add Task    ${file}
    END

Enroll Crinit Root Key
    ${key_id}    Execute And Log Based On User Permissions
    ...          sh -c "cat ${PUBKEY_PATH} | keyctl padd user crinit-root @s"    ${RETURN_STDOUT}
    Should Not Be Empty    ${key_id}
    ${rc}        Execute And Log Based On User Permissions
    ...          keyctl setperm ${key_id} 0x3f3f0000   ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}        Execute And Log Based On User Permissions
    ...          keyctl link ${key_id} @u   ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
    ${rc}        Execute And Log Based On User Permissions
    ...          keyctl unlink ${key_id} @s   ${RETURN_RC}
    Should Be Equal As Numbers  ${rc}  0
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
