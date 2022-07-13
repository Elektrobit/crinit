#!/bin/bash -e
#
# script to run smoketests for integration
#
# Dependency: ci/build.sh needs to be run before
#
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# architecture name amd64, arm64, ...
ARCH=$(dpkg --print-architecture)

BINDIR=${BASEDIR}/result/"$ARCH"/bin
LIBDIR=${BASEDIR}/result/"$ARCH"/lib
CONFDIR=${BASEDIR}/config/test
export LD_LIBRARY_PATH="${LIBDIR}"

SMOKETESTS_REPORT="$BASEDIR"/result/"$ARCH"/smoketests_report.txt
SMOKETESTS_LOG="$BASEDIR"/result/"$ARCH"/smoketests.log

# check if ci/build.sh has been run before
if [ ! -d "$BASEDIR/result/$ARCH" ]; then
    echo Build environment not set up. Please run ci/build.sh first!
    exit 1
fi

rm -f "$SMOKETESTS_REPORT"
rm -f "$SMOKETESTS_LOG"

cd "$BASEDIR"

source test/smoketests/lib.sh

source test/smoketests/startstop.sh
