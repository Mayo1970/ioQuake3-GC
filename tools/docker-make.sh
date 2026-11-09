#!/usr/bin/env bash
set -euo pipefail

IMAGE="devkitpro/devkitppc:20261102"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export MSYS_NO_PATHCONV=1

docker run --rm \
	-v "${REPO_ROOT}:/src" \
	-w /src \
	"${IMAGE}" \
	bash -c 'export DEVKITPRO=/opt/devkitpro DEVKITPPC=/opt/devkitpro/devkitPPC PATH=/opt/devkitpro/devkitPPC/bin:/opt/devkitpro/tools/bin:$PATH; make "$@"' -- "$@"
