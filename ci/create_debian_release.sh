#!/bin/bash
set -euo pipefail

OUTDIR=/base/debianbuild

function print_help() {
    echo "$0 [-r|-p|-a|-h] <x.y.z>"
    echo -e "-r\tonly do the debian/main release"
    echo -e "-p\tonly do the pristine tar build and update"
    echo -e "-a\tdo all (default)"
    echo -e "-h\tprint help"
    echo ""
    echo "It is necessary to have git's author information correctly set."
    echo "Either set it with 'git config' or provide it via environment variables:"
    echo "* GIT_AUTHOR_NAME"
    echo "* GIT_AUTHOR_EMAIL"
}

function setup_env() {
    GLOBMAIL=$(git config --global user.email || true)
    GLOBNAME=$(git config --global user.name || true)

    LOCMAIL=$(git config --local user.email || true)
    LOCNAME=$(git config --local user.name || true)

    TMPNAME="${GIT_AUTHOR_NAME:-${LOCNAME:-${GLOBNAME:-}}}"

    if [ -z "${TMPNAME}" ]; then
        echo "Invalid environment. Could not determine author name for git"
        exit 1
    fi

    TMPMAIL="${GIT_AUTHOR_EMAIL:-${LOCMAIL:-${GLOBMAIL:-}}}"

    if [ -z "${TMPMAIL}" ]; then
        echo "Invalid environment. Could not determine author e-mail for git"
        exit 1
    fi

    export GIT_AUTHOR_EMAIL="${TMPMAIL}"
    export GIT_COMMITTER_EMAIL="${TMPMAIL}"
    export EMAIL="${TMPMAIL}"
    export GIT_AUTHOR_NAME="${TMPNAME}"
    export GIT_COMMITTER_NAME="${TMPNAME}"
    export NAME="${TMPNAME}"

    export DEBNAME="${NAME}"
    export DEBMAIL="${EMAIL}"

    export DEBIAN_FRONTEND=noninteractive
}

function create_and_publish_debian_main() {
    gbp import-ref -u "${NEW_VERSION}" --debian-branch "$(git branch --show-current)"

    dch -D unstable --newversion="${NEW_VERSION}-1" "New upstream tag ${NEW_VERSION}"
    git add debian/ && git commit -m "New upstream tag ${NEW_VERSION}"
    git checkout HEAD && git clean -fxd
}

function create_and_publish_pristine_tar() {
    gbp buildpackage --git-compression=xz -uc -us --git-export-dir="${OUTDIR}" --git-ignore-branch
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
