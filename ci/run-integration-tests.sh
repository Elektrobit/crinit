#!/bin/bash

CMD_PATH=$(cd $(dirname $0) && pwd)
BASE_DIR=${CMD_PATH%/*}

BUILD_TYPE="${1:-Release}"

# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)
UBUNTU_RELEASE="jammy"

case "$BUILD_TYPE" in
    Release)
        BUILD_DIR="$BASE_DIR/build/$ARCH"
        RESULT_DIR="$BASE_DIR/result/$ARCH"
        ;;
    *)
        BUILD_DIR="$BASE_DIR/build/$ARCH-$BUILD_TYPE"
        RESULT_DIR="$BASE_DIR/result/$ARCH-$BUILD_TYPE"
        ;;
esac


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

PROJECT=${BASE_DIR##*/}
CRINIT_IMAGE_NAME="$(clean_tag "${PROJECT}-crinit")"
CRINIT_DOCKER_NAME="$(clean_tag "${PROJECT}-target")"
TEST_IMAGE_NAME="$(clean_tag "${PROJECT}-robot")"
TEST_DOCKER_NAME="$(clean_tag "${PROJECT}-runner")"
BUILD_ARG=${BUILD_ARG:-}

# map architecture to Docker per-architecture repositories
# https://github.com/docker-library/official-images#architectures-other-than-amd64
REPO="$ARCH"
if [ "$ARCH" = "arm64" ]; then
    REPO=arm64v8
    PLATFORM_OPTS="--platform linux/arm64/v8"
fi

if [ -n "${CI}" ]; then
    BUILD_ID="${BUILD_ID:-'none'}"
    GIT_COMMIT="${GIT_COMMIT:-'none'}"
    CRINIT_IMAGE_NAME="$(clean_tag "${CRINIT_IMAGE_NAME}${ARCH:+-}${ARCH}-${BUILD_ID}-${GIT_COMMIT}")"
    CRINIT_DOCKER_NAME="$(clean_tag "${CRINIT_DOCKER_NAME}${ARCH:+-}${ARCH}-${BUILD_ID}-${GIT_COMMIT}")"
    TEST_IMAGE_NAME="$(clean_tag "${TEST_IMAGE_NAME}${ARCH:+-}${ARCH}-${BUILD_ID}-${GIT_COMMIT}")"
    TEST_DOCKER_NAME="$(clean_tag "${TEST_DOCKER_NAME}${ARCH:+-}${ARCH}-${BUILD_ID}-${GIT_COMMIT}")"
fi

rm -rf $BUILD_DIR/result/integration
mkdir -p $BUILD_DIR/result/integration

DOCKER_BUILDKIT=1 \
docker build \
    $BUILD_ARG \
    --ssh default \
    --progress=plain \
    --build-arg REPO="$REPO" \
    --build-arg UBUNTU_RELEASE="$UBUNTU_RELEASE" \
    --build-arg UID=$(id -u) --build-arg GID=$(id -g) \
    --tag ${CRINIT_IMAGE_NAME} -f $BASE_DIR/ci/Dockerfile.crinit .

docker build \
    $BUILD_ARG \
    --build-arg UID=$(id -u) --build-arg GID=$(id -g) \
    --tag ${TEST_IMAGE_NAME} -f $BASE_DIR/ci/Dockerfile.itest .

if [ "$SSH_AUTH_SOCK" ]; then
    SSH_AGENT_SOCK=$(readlink -f $SSH_AUTH_SOCK)
    SSH_AGENT_OPTS="-v $SSH_AGENT_SOCK:/run/ssh-agent -e SSH_AUTH_SOCK=/run/ssh-agent"
fi

CRINIT_ID=$(docker run -d -ti --rm \
    --name ${CRINIT_DOCKER_NAME} \
    --cap-add=SYS_ADMIN \
    --security-opt apparmor=unconfined \
    $SSH_AGENT_OPTS \
    --privileged \
    -w / \
    ${CRINIT_IMAGE_NAME})

RUNNER_ID=$(docker run -d -ti --rm \
    --name ${TEST_DOCKER_NAME} \
    --link ${CRINIT_DOCKER_NAME} \
    -v $BASE_DIR:/base \
    -w /base \
    --env "PROJECT=${PROJECT}" \
    --env "TEST_OUTPUT=/base/build/${BUILD_TYPE}/result/integration" \
    ${TEST_IMAGE_NAME})

docker exec ${TEST_DOCKER_NAME} /base/test/integration/scripts/run-integration-tests.sh

ret=$?

docker stop ${CRINIT_ID}
docker stop ${RUNNER_ID}

cp -a "${BUILD_DIR}/result/integration" "${RESULT_DIR}/"

exit $?
