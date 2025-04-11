#!/bin/bash -eu
# SPDX-License-Identifier: MIT
#
CMD_PATH=$(cd "$(dirname "$0")" && pwd)
BASE_DIR=${CMD_PATH%/*}

function printHelp() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo -e "\t -c\t\tclean output directory and generated files before building"
    echo -e "\t -h\t\tprint this help and exit"
}

OPTION_CLEAN=0
while ((${#})); do
    ARG="${1:-}"
    case "${ARG}" in
        -h | --help)
            printHelp
            exit 1
            ;;
        -c | --clean)
            OPTION_CLEAN=1
            ;;
        -*)
            printf "Unexpected option: %s\\n\\n" "${ARG}"
            printHelp
            exit 1
            ;;
    esac
    shift
done

ARCH=$(dpkg --print-architecture)
if [ "$ARCH" != "amd64" ]; then
    exit 0 # only need amd64 documentation, and building it for arm64 produces complicated to fix errors.
fi
BUILD_DIR="$BASE_DIR/build/$ARCH"
RESULT_DIR="$BASE_DIR/result/$ARCH"
DIST_DIR="$BUILD_DIR/dist"

CRINIT_SOURCE_SOURCE_DIR=${BASE_DIR}/src
CRINIT_SOURCE_HEADER_DIR=${BASE_DIR}/inc

SPHINX_SOURCE_DIR=${BASE_DIR}
SPHINX_BUILD_DIR=${RESULT_DIR}/doc/sphinx
SPHINX_GENERATED_SOURCE_DIR=${SPHINX_BUILD_DIR}/source_generated
SPHINX_HTML_OUTPUT_DIR=${SPHINX_BUILD_DIR}/html

# shellcheck disable=SC1091 # Shellcheck does not need to follow and lint the venv script.
. "${SPHINX_VENV-${BASE_DIR}/.venv/}"/bin/activate

function createApiDocu() {
    sphinx-c-apidoc --force \
        -o "${SPHINX_GENERATED_SOURCE_DIR}/api/crinit" \
        --tocfile sources \
        "${CRINIT_SOURCE_SOURCE_DIR}/"
    sphinx-c-apidoc --force \
        -o "${SPHINX_GENERATED_SOURCE_DIR}/api/crinit" \
        --tocfile header \
        "${CRINIT_SOURCE_HEADER_DIR}/"

    echo -e "
Crinit API
==========================

.. toctree::
  :maxdepth: 1
  :caption: Contents:

  crinit sources <crinit/sources>
  crinit header <crinit/header>
" >"${SPHINX_GENERATED_SOURCE_DIR}/api/index.rst"
}

if [ ${OPTION_CLEAN} -eq 1 ]; then
    echo "Delete ${SPHINX_GENERATED_SOURCE_DIR} ${SPHINX_BUILD_DIR}"
    rm -rf "${SPHINX_GENERATED_SOURCE_DIR}" "${SPHINX_BUILD_DIR}"
fi

mkdir -p "${SPHINX_BUILD_DIR}" "${SPHINX_GENERATED_SOURCE_DIR}/ADRs" "${SPHINX_GENERATED_SOURCE_DIR}/developer"

createApiDocu

function resetReadMeImages() {
    sed -i 's#src="_static/#src="images/#' README.md
}
sed -i 's#src="images/#src="_static/#' README.md
trap resetReadMeImages EXIT

export PATH="${PATH}:${DIST_DIR}/usr/local/bin"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH-"./"}:${DIST_DIR}/usr/local/lib"
if ! sphinx-build -b html \
    "${SPHINX_SOURCE_DIR}" \
    "${SPHINX_HTML_OUTPUT_DIR}" \
    2>"${SPHINX_BUILD_DIR}/html_doc_error.log"; then
    echo "Build failed, for details see ${SPHINX_BUILD_DIR}/html_doc_error.log"
    exit 1
fi

WARNINGS=$(grep -c -e ": WARNING:" -e ": ERROR:" "${SPHINX_BUILD_DIR}/html_doc_error.log" || true)
if [ "${WARNINGS}" -ne 0 ]; then
    echo ""
    echo "Build warnings ${WARNINGS}"
    echo ""
    cat "${SPHINX_BUILD_DIR}/html_doc_error.log"
    exit 1
fi
