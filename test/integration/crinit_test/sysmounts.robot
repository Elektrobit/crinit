# SPDX-License-Identifier: MIT
*** Settings ***
Documentation     A test suite to check if the --sys-mounts CLI option behaves correctly.

Resource          ../keywords.resource
Resource          ../elosd-keywords.resource
Resource          ../crinit-keywords.resource

Library           String
Library           SSHLibrary

Suite Setup       Connect To Target
Suite Teardown    Disconnect From Target

*** Test Cases ***
Crinit Mounts System Directories If Option Activated
    [Documentation]    "Crinit creates hard coded system directories if --sys-mounts parameter is given."
    [Setup]    Set Up A Test Case With Sysmount Option Activated
    Check Mount Type Of Path    "${TEST_CHROOT}/proc "     proc
    Check Mount Type Of Path    "${TEST_CHROOT}/sys "      sysfs
    Check Mount Type Of Path    "${TEST_CHROOT}/dev "      devtmpfs
    Check Mount Type Of Path    "${TEST_CHROOT}/dev/pts "  devpts
    Check Mount Type Of Path    "${TEST_CHROOT}/run "      tmpfs
    [Teardown]    Clean Up Test Case With Sysmount Option Activated

Crinit Does Not Mount System Directories If Option Deactivated
    [Documentation]    "Crinit does not create hard coded system directories if --no-sys-mounts parameter is given."
    [Setup]    Set Up A Test Case With Sysmount Option Deactivated
    # Unfortunately we won't be able to check /proc and /run as well as we need to mount them manually for Crinit to
    # start. Nevertheless we can check if Crinit has erroneously mounted the remaining three.
    Check If Path Not Mounted    "${TEST_CHROOT}/sys "
    Check If Path Not Mounted    "${TEST_CHROOT}/dev "
    Check If Path Not Mounted    "${TEST_CHROOT}/dev/pts "
    [Teardown]    Clean Up Test Case With Sysmount Option Deactivated

*** Keywords ***
Connect To Target
    Connect To Target And Log In

Disconnect From Target
    Close All Connections

Do Chroot Preparations
    ${stdout}  Execute And Log Based On User Permissions  mktemp -d  ${RETURN_STDOUT}
    Should Not Be Empty    ${stdout}
    Set Test Variable    ${TEST_CHROOT}    ${stdout}
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/bin"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/bin" "${TEST_CHROOT}/bin"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/usr"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/usr" "${TEST_CHROOT}/usr"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/lib"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/lib" "${TEST_CHROOT}/lib"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/tmp"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/tmp" "${TEST_CHROOT}/tmp"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/etc"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/etc" "${TEST_CHROOT}/etc"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/lib" "${TEST_CHROOT}/lib"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    # These next two bind mounts may fail depending on system conf. That's fine.
    Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/lib32"
    Should Be Equal As Numbers    ${rc}    0
    Execute And Log Based On User Permissions  mount --bind "/lib32" "${TEST_CHROOT}/lib32"
    Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/lib64"
    Should Be Equal As Numbers    ${rc}    0
    Execute And Log Based On User Permissions  mount --bind "/lib64" "${TEST_CHROOT}/lib64"

Do Chroot Cleanup
    Should Not Be Empty    ${TEST_CHROOT}
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/bin"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/usr"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/lib"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/tmp"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/etc"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/lib"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    # These next two may fail depending on system conf and test variant. That's fine.
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/lib32"
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/lib64"
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/proc"
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/run"
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/systest/fs/cgroup"
    Execute And Log Based On User Permissions  umount -l "${TEST_CHROOT}/systest"
    Execute And Log Based On User Permissions  rm -rf "${TEST_CHROOT}/systest"

Set Up A Test Case With Sysmount Option Activated
    Do Chroot Preparations
    Crinit Start  chroot=${TEST_CHROOT}  crinit_args=--sys-mounts

Set Up A Test Case With Sysmount Option Deactivated
    Do Chroot Preparations
    # We also need to bind-mount /proc and /run as Crinit will crash if its not available.
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/proc"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/proc" "${TEST_CHROOT}/proc"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/run"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/run" "${TEST_CHROOT}/run"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/sys/fs"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mkdir -p "${TEST_CHROOT}/systest"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/sys" "${TEST_CHROOT}/systest"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  mount --bind "/sys/fs/cgroup" "${TEST_CHROOT}/systest/fs/cgroup"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    ${rc}  Execute And Log Based On User Permissions  ln -s "/systest/fs/cgroup" "${TEST_CHROOT}/sys/fs/"  ${RETURN_RC}
    Should Be Equal As Numbers    ${rc}    0
    Crinit Start  chroot=${TEST_CHROOT}  crinit_args=--no-sys-mounts

Clean Up Test Case With Sysmount Option Activated
    Do Chroot Cleanup
    Crinit Stop   crinit_args=--sys-mounts

Clean Up Test Case With Sysmount Option Deactivated
    Do Chroot Cleanup
    Crinit Stop   crinit_args=--no-sys-mounts

Check Mount Type Of Path
    [Arguments]    ${Path}=    ${Type}=
    ${stdout}  Execute And Log Based On User Permissions
    ...    cat /proc/mounts | grep ${Path} | cut -f3 -d ' '    ${RETURN_STDOUT}
    Should Be Equal As Strings    ${stdout.strip()}   ${Type.strip()}

Check If Path Not Mounted
    [Arguments]    ${Path}=
    ${stdout}  Execute And Log Based On User Permissions
    ...    cat /proc/mounts | grep ${Path}    ${RETURN_STDOUT}
    Should Be Empty    ${stdout.strip()}
