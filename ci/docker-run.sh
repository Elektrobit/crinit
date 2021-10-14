#!/bin/sh -e
#
# create and run docker build env
#
CMDPATH=$(cd $(dirname $0) && pwd)
BASEDIR=${CMDPATH%/*}
PROJECT=eb-baseos-crinit

echo "==> create docker image"
cd $BASEDIR/ci
docker build \
    --build-arg UID=$(id -u) --build-arg GID=$(id -g) \
    --tag $PROJECT .

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
docker run --rm -it --privileged $SSH_AGENT_OPTS \
    -v $HOME/.ssh:/home/ci/.ssh -v $BASEDIR/ci/sshconfig:/home/ci/.ssh/config \
    -v $BASEDIR:/base -w /base $PROJECT $@

