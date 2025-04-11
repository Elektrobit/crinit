#!/bin/sh -e
#
# create and run docker build env
#
CMD_PATH=$(cd "$(dirname "$0")" && pwd)
BASE_DIR=${CMD_PATH%/*}
PROJECT=${BASE_DIR##*/}

clean_tag() {
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

CRINIT_DOCKER_NAME="$(clean_tag "${PROJECT}-target")"
TEST_IMAGE_NAME="$(clean_tag "${PROJECT}-robot")"
TEST_DOCKER_NAME="$(clean_tag "${PROJECT}-runner")"

echo "==> create docker image"
cd "$BASE_DIR/ci"
docker build \
    --build-arg UID="$(id -u)" --build-arg GID="$(id -g)" \
    --tag "${TEST_IMAGE_NAME}" -f Dockerfile.itest .

echo "==> run $PROJECT robot container"

if ! [ -e "$BASE_DIR"/ci/sshconfig ]; then
    {
        echo "Host *"
        echo "  User $(id -u -n)"
    } >"$BASE_DIR"/ci/sshconfig
fi

if [ "$SSH_AUTH_SOCK" ]; then
    SSH_AGENT_SOCK=$(readlink -f "$SSH_AUTH_SOCK")
    SSH_AGENT_OPTS="-v $SSH_AGENT_SOCK:/run/ssh-agent -e SSH_AUTH_SOCK=/run/ssh-agent"
fi

# shellcheck disable=SC2086 # Intended splitting of SSH_AGENT_OPS.
docker run --rm -it $SSH_AGENT_OPTS \
    --name "${TEST_DOCKER_NAME}" \
    --link "${CRINIT_DOCKER_NAME}" \
    -v "$BASE_DIR/..:/base" \
    -w /base \
    "${TEST_IMAGE_NAME}" "$@"
