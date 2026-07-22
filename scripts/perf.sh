#!/bin/bash

set -eE

BASE_DIR="$(dirname `readlink -f ${0}`)"
BUILD_DIR="$(readlink -f "${BASE_DIR}/../build")"
FLAMEGRAPH_DIR="${BUILD_DIR}/FlameGraph"

function cleanup()
{
    [ -n "${OUT_PERF}" ]   && rm -rf "${OUT_PERF}"
    [ -n "${OUT_FOLDED}" ] && rm -rf "${OUT_FOLDED}"
    [ -f perf.data ]       && rm -rf perf.data
}

trap cleanup EXIT

PERF_COMMAND="${1}"
shift

case "${PERF_COMMAND}" in
    flamegraph)
        mkdir -p "${BUILD_DIR}"
        if [ ! -d "${FLAMEGRAPH_DIR}" ]
        then
            git clone --revision 41fee1f99f9276008b7cd112fca19dc3ea84ac32 --depth 1 https://github.com/brendangregg/FlameGraph "${FLAMEGRAPH_DIR}"
        fi
        OUT_PERF="$(mktemp)"
        OUT_FOLDED="$(mktemp)"
        perf record ${@}
        perf script > "${OUT_PERF}"
        ${FLAMEGRAPH_DIR}/stackcollapse-perf.pl "${OUT_PERF}" > "${OUT_FOLDED}"
        ${FLAMEGRAPH_DIR}/flamegraph.pl "${OUT_FOLDED}" > "${BUILD_DIR}/flamegraph.svg"
        echo "Flamegraph: ${BUILD_DIR}/flamegraph.svg"
        ;;
    *)
        perf "${PERF_COMMAND}" ${@}
        ;;
esac
