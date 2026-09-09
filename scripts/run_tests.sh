#!/bin/sh

# Configures, builds and runs tests/CMakeLists.txt.
# An argument of "debug" builds and runs the debug configuration; the default is release.

set -eu

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONFIG=Release
if [ "${1:-}" = debug ]; then CONFIG=Debug; fi

BUILD_DIR="${ROOT}/build/${CONFIG}"

cmake -S "${ROOT}/tests" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${CONFIG}" -DOUTPUT_DIR="${BUILD_DIR}/bin"
cmake --build "${BUILD_DIR}" --config "${CONFIG}" --parallel "$(getconf _NPROCESSORS_ONLN)"

exec "${BUILD_DIR}/bin/tests"
