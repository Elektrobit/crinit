#!/bin/bash
set -euo pipefail

function print_help() {
    echo "$0 [-r|-p|-a|-h] <x.y.z>"
    echo -e "-r\tonly do the debian/main release"
    echo -e "-p\tonly do the pristine tar build and update"
    echo -e "-a\tdo all (default)"
    echo -e "-h\tprint help"
    echo ""
    echo "It is necessary to have a propper configured environment with:"
    echo "* GIT_AUTHOR_NAME"
    echo "* GIT_AUTHOR_EMAIL"
}

function setup_env() {
    export GIT_COMMITTER_NAME="${GIT_AUTHOR_NAME}"
    export GIT_COMMITTER_EMAIL="${GIT_AUTHOR_EMAIL}"

    export NAME="${GIT_AUTHOR_NAME}"
    export EMAIL="${GIT_AUTHOR_EMAIL}"

    export DEBNAME="${GIT_AUTHOR_NAME}"
    export DEBMAIL="${GIT_AUTHOR_EMAIL}"

    git config --local user.name "${GIT_AUTHOR_NAME}"
    git config --local user.email "${GIT_AUTHOR_EMAIL}"

    export DEBIAN_FRONTEND=noninteractive
    sudo ln -fs /usr/share/zoneinfo/UTC /etc/localtime

    sudo apt-get install -y \
        debhelper \
        devscripts \
        equivs \
        fakeroot \
        git \
        git-buildpackage \
        software-properties-common
    sudo add-apt-repository -y ppa:elos-team/ppa
    sudo apt-get install -y libsafu-dev
}

function create_and_publish_debian_main() {
    gbp import-ref -u "${NEW_VERSION}" --debian-branch "$(git branch --show-current)"

    dch -D unstable --newversion="${NEW_VERSION}-1" "New upstream tag ${NEW_VERSION}"
    git add debian/ && git commit -m "New upstream tag ${NEW_VERSION}"
    git checkout HEAD && git clean -fxd
}

function create_and_publish_pristine_tar() {
    sudo gbp buildpackage --git-compression=xz -uc -us
}

UPDATE_RELEASE=0
UPDATE_PRISTINE=0
UPDATE_ALL=1

while getopts "raph" opt; do
    case $opt in
        r) UPDATE_RELEASE=1 ;;
        p) UPDATE_PRISTINE=1 ;;
        a) UPDATE_ALL=1 ;;
        h)
            print_help
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            print_help
            exit 1
            ;;
    esac
done
NEW_VERSION=${!OPTIND}

if [ -z "$NEW_VERSION" ]; then
    echo "Error: Version number is required."
    print_help
    exit 1
fi

if [ $UPDATE_RELEASE -eq 0 ] && [ $UPDATE_PRISTINE -eq 0 ]; then
    UPDATE_ALL=1
else
    UPDATE_ALL=0
fi

echo "Create release: ${NEW_VERSION}"

setup_env

if [ $UPDATE_ALL -eq 1 ] || [ $UPDATE_RELEASE -eq 1 ]; then
    create_and_publish_debian_main
fi

if [ $UPDATE_ALL -eq 1 ] || [ $UPDATE_PRISTINE -eq 1 ]; then
    create_and_publish_pristine_tar
fi
