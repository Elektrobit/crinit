#!/bin/sh
# SPDX-License-Identifier: MIT
#
# Ensure that the version numbers in CMakeLists.txt
# and RPM spec file are the same.
#

CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

INPUTFILE_CMAKE="$BASEDIR/CMakeLists.txt"
INPUTFILE_RPM="$BASEDIR/packaging/nautilos/crinit.spec"

# Extract version from CMakeLists.txt
VERSION_CMAKE=$(sed -nE 's/^project\(.* VERSION (.*)\)$/\1/p' "$INPUTFILE_CMAKE")

# Extract version from RPM spec file
VERSION_RPM=$(sed -nE 's/^Version: (.*)$/\1/p' "$INPUTFILE_RPM")

if [ "$VERSION_CMAKE" != "$VERSION_RPM" ]; then
    echo "Error: version mismatch detected!" >&2
    echo "  ${INPUTFILE_CMAKE#$BASEDIR/*}: $VERSION_CMAKE" >&2
    echo "  ${INPUTFILE_RPM#$BASEDIR/*}: $VERSION_RPM" >&2
    exit 1
fi

exit 0
