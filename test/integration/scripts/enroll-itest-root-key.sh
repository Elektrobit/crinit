#!/bin/sh
# SPDX-License-Identifier: MIT

print_usage() {
    echo "Script to enroll the crinit root key in the Kernel keyring, used by the robot tests to prepare the system."
    echo "USAGE: $0 <path/to/rootkey.der>"
    echo "The script will print the ID of the newly enrolled key to stdout."
}

if [ ! -f "$1" ]; then
    print_usage()
    exit 1
fi

set -e

KEY_ID=$(cat "$1" | keyctl padd user crinit-root @s)
keyctl setperm "${KEY_ID}" 0x3f3f0000 > /dev/null
keyctl link "${KEY_ID}" @u > /dev/null
keyctl unlink "${KEY_ID}" @s > /dev/null
echo "${KEY_ID}"
