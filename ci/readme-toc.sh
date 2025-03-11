#!/bin/bash
# SPDX-License-Identifier: MIT
#
# README table of contents generation and lint script for merge request
# branches.
#
# Usage: ./ci/readme-toc.sh
#
# This will update the README Table of Contents automatically. If git detects
# changes afterwards, the script will return 1, otherwise 0.
#
# Set DOCTOC_CMD env variable if your doctoc binary/script is not in
# your path or if you want to run using npx.
#

set -euo pipefail

DOCTOC_CMD=${DOCTOC_CMD:-doctoc}
CMDPATH=$(cd "$(dirname "$0")" && pwd)
BASEDIR=${CMDPATH%/*}

# Update README ToC.
${DOCTOC_CMD} --github "${BASEDIR}/README.md"

# Check if the Update changed anything and report via exit status (useful for
# the pipeline check). This will be false positive if there are other unstaged
# changes in the README but that does not matter for development and should
# not happen in the pipeline.
git diff --quiet
