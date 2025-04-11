#!/usr/bin/sh -e
# SPDX-License-Identifier: MIT
#
# Run Robot tests using this script.
#

CMD_PATH=$(cd "$(dirname "$0")" && pwd)
BASE_DIR=${CMD_PATH%/*}
PROJECT="${PROJECT-${BASE_DIR##*/}}"
TEST_SOURCE=${TEST_SOURCE-${CMD_PATH%/*}}
VARIABLE_FILE="${VARIABLE_FILE-${TEST_SOURCE}/robot_variables.py}"
TEST_OUTPUT="${TEST_OUTPUT-${TEST_SOURCE}/report}"
TEST_DIR="_test"

clean_hostname() {
    if [ -z "${1}" ]; then
        echo "Error: Argument for clean_tag missing."
        exit 1
    fi

    echo "${1}" \
        | tr -c '[:alnum:].-' '-' \
        | tr '[:upper:]' '[:lower:]' \
        | sed 's/^[^[:alnum:]]*//;s/[^[:alnum:]]*$//' \
        | sed 's/-\{2,\}/-/g'
}

TARGET_NAME="$(clean_hostname "${TARGET_NAME-${PROJECT}-target}")"
TARGET_IP=$(grep "${TARGET_NAME}" /etc/hosts | cut -f 1)

if [ -z "${TARGET_IP}" ]; then
    TARGET_IP=$(ping -c 2 "${TARGET_NAME}" | awk -F '[()]' '/PING/ { print $2}')
fi

if [ -e "${VARIABLE_FILE}" ]; then
    if grep -q "TARGET_IP" "${VARIABLE_FILE}"; then
        sed -i "/TARGET_IP/c\TARGET_IP = \"$TARGET_IP\"" "$VARIABLE_FILE"
    fi
else
    echo "robot_variables file does not exist"
    exit 1
fi

for dir in "$TEST_SOURCE"/*/; do
    if [ "${dir#*"$TEST_DIR"}" != "$dir" ]; then
        TEST_NAME=$(echo "$dir" | rev | cut -d '/' -f 2 | rev)
        printf '\nStart test suite %s\n' "$dir"
        robot --variablefile="$VARIABLE_FILE" --outputdir="$TEST_OUTPUT"/"$TEST_NAME" --output="$TEST_NAME" --report="$TEST_NAME" "$dir" || exit_status=$?
    fi
done

exit "${exit_status:-0}"
