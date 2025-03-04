#!/bin/sh
# SPDX-License-Identifier: MIT

print_usage() {
    echo "Script to enroll the crinit root key in the Kernel keyring, used by the robot tests to prepare the system."
    echo "USAGE: keyctl session - $0 <path/to/rootkey.der>"
    echo "The script will print the ID of the newly enrolled key to stdout."
}

if [ ! -f "$1" ]; then
    print_usage() exit 1
fi

set -e

KEY_ID=$(cat "$1" | keyctl padd user crinit-root @s)

# As noted in the discussions of https://github.com/systemd/systemd/issues/5522, the Kernel user keyring seems to block
# setting permissions for keys even if you own them. To counter this, systemd automatically links the session and user
# keyrings during startup since https://github.com/systemd/systemd/pull/6275.
#
# As neither Crinit nor most other init systems include this workaround (unless explicitly set in a script by the user),
# we should assume the user keyring to not be usable with keyctl. Luckily, https://mjg59.dreamwidth.org/37333.html
# details a way around that. We can simply create the key in the session keyring, set permissions and then move it to
# the user keyring.
#
# In case, the current session keyring has not been initialized, it is recommended to run this script in a new keyring
# session, via `keyctl session - enroll-itest-root-key.sh`.
#
keyctl setperm "${KEY_ID}" 0x3f3f0000 >/dev/null
keyctl link "${KEY_ID}" @u >/dev/null
keyctl unlink "${KEY_ID}" @s >/dev/null
echo "${KEY_ID}"
