#!/bin/sh -e
# SPDX-License-Identifier: MIT
#
# create and run docker build env
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}
PROJECT=eb-baseos-crinit
ARCH="amd64"

if [ -n "$1" ]; then
    ARCH="$1"
    shift
fi

# map architecture to Docker per-architecture repositories
# https://github.com/docker-library/official-images#architectures-other-than-amd64
REPO="$ARCH"
if [ "$ARCH" = "arm64" ]; then
    REPO=arm64v8
    PLATFORM_OPTS="--platform linux/arm64/v8"
fi

IMAGE="${PROJECT}${ARCH:+-}${ARCH}"

echo "==> create docker image"
cd $BASEDIR/ci
docker build \
    --build-arg REPO="$REPO" \
    --build-arg UID=$(id -u) --build-arg GID=$(id -g) \
    --tag "${IMAGE}" .

if ! [ -e "$BASEDIR/ci/sshconfig" ]; then
    { echo "Host *"
      echo "  User $(id -u -n)"
    } > $BASEDIR/ci/sshconfig
fi

if [ "$SSH_AUTH_SOCK" ]; then
    SSH_AGENT_SOCK=$(readlink -f $SSH_AUTH_SOCK)
    SSH_AGENT_OPTS="-v $SSH_AGENT_SOCK:/run/ssh-agent -e SSH_AUTH_SOCK=/run/ssh-agent"
fi

echo "==> run $PROJECT build container"
docker run --rm -it --privileged \
    $SSH_AGENT_OPTS \
    $PLATFORM_OPTS \
    -v $HOME/.ssh:/home/ci/.ssh -v $BASEDIR/ci/sshconfig:/home/ci/.ssh/config \
    -v $BASEDIR:/base -w /base "$IMAGE" $@

