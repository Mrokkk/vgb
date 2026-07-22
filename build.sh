#!/bin/bash

set -e

BASE_DIR="$(dirname `readlink -f ${0}`)"
SRC_DIR="$(readlink -f "${BASE_DIR}")"
BUILD_DIR=.
REGENERATE=

RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[34m"
ERROR="[ ERROR   ]"
SUCCESS="[ SUCCESS ]"
INFO="[  INFO   ]"
DEBUG="[  DEBUG  ]"
RESET="\e[m"

info()
{
    echo -e "${BLUE}${INFO}${RESET} ${@}"
}

error()
{
    echo -e "${RED}${ERROR}${RESET} ${@}"
}

die()
{
    echo -e "${RED}${ERROR} ${@}${RESET}"
    exit 1
}

pushd_silent()
{
    pushd "${1}" &>/dev/null || die "No directory: ${1}"
}

popd_silent()
{
    popd &>/dev/null || die "Cannot go back to previous dir!"
}

read_cache()
{
    if [ -f "${BUILD_DIR}/CMakeCache.txt" ]
    then
        while IFS= read -r line
        do
            case ${line} in
                "OPTIMIZE:"*)
                    OPTIMIZE="${line#*=}"
                    ;;
                "LTO:"*)
                    LTO="${line#*=}"
                    ;;
                "COVERAGE:"*)
                    COVERAGE="${line#*=}"
                    ;;
                "SANITIZE:"*)
                    SANITIZE="${line#*=}"
                    ;;
                "BUILD_TESTS:"*)
                    BUILD_TESTS="${line#*=}"
                    ;;
                *)
                    ;;
            esac
        done <<< $(cat ${BUILD_DIR}/CMakeCache.txt)
    fi
}

read_flags()
{
    local temp
    while [ $# -gt 0 ]; do
        case "${1}" in
            --optimize=*)
                temp="${1#*=}"
                if [ "${temp}" != "${OPTIMIZE}" ]
                then
                    REGENERATE="true"
                fi
                OPTIMIZE="${temp}"
                ;;
            --lto=*)
                temp="${1#*=}"
                if [ "${temp}" != "${LTO}" ]
                then
                    REGENERATE="true"
                fi
                LTO="${temp}"
                ;;
            --coverage=*)
                temp="${1#*=}"
                if [ "${temp}" != "${COVERAGE}" ]
                then
                    REGENERATE="true"
                fi
                COVERAGE="${temp}"
                ;;
            --sanitize=*)
                temp="${1#*=}"
                if [ "${temp}" != "${SANITIZE}" ]
                then
                    REGENERATE="true"
                fi
                SANITIZE="${temp}"
                ;;
            --profile=*)
                temp="${1#*=}"
                if [ "${temp}" != "${PROFILE}" ]
                then
                    REGENERATE="true"
                fi
                PROFILE="${temp}"
                ;;
            --build-tests=*)
                temp="${1#*=}"
                if [ "${temp}" != "${BUILD_TESTS}" ]
                then
                    REGENERATE="true"
                fi
                BUILD_TESTS="${temp}"
                ;;
            -j*)
                NINJA_FLAGS+="${1}"
                ;;
            build|run|test)
                COMMAND="${1}"
                shift
                ARGS=("$@")
                break
                ;;
            *)
                die "Unsupported flag/command: ${1}"
                ;;
        esac
        shift
    done
}

regenerate_cmake()
{
    if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/build.ninja" ] || [ -n "${REGENERATE}" ]
    then
        cmake \
            -DOPTIMIZE=${OPTIMIZE} \
            -DLTO=${LTO} \
            -DSANITIZE=${SANITIZE} \
            -DCOVERAGE=${COVERAGE} \
            -DBUILD_TESTS=${BUILD_TESTS} \
            -DPROFILE=${PROFILE} \
            -GNinja \
            -B "${BUILD_DIR}" "${SRC_DIR}"
    fi
}

build_target()
{
    set -e
    pushd_silent "${BUILD_DIR}"
    if [ -f build.ninja ]
    then
        local ts="$(date +%s)"
        ninja ${@}
        info "Build stats:"
        ${BASE_DIR}/scripts/ninja_log_parse.py ".ninja_log" "${ts}"
    else
        die "CMake build was not generated"
    fi
    popd_silent
}

run_command()
{
    local critical=
    local time_cmd=

    case "${1}" in
        -t)
            shift
            time_cmd=on
            ;;
        -e)
            shift
            critical=on
            ;;
        *)
            ;;
    esac

    info "Running: ${@}"
    local command="${1}"
    shift

    if [ "${SANITIZE}" == "ON" ]
    then
        rm -rf /tmp/asan.log*
        export ASAN_OPTIONS="color=always:log_path=/tmp/asan.log:abort_on_error=1"
    fi

    set +e

    local wrapper=

    if [ -n "${time_cmd}" ]
    then
        if [ -f /usr/bin/time ]
        then
            wrapper="/usr/bin/time -v"
        else
            wrapper="time"
        fi
    fi

    ${wrapper} "${command}" "${@}"

    status=$?

    if [ ${status} -ge 128 ]
    then
        error "Command killed by signal $((status - 128))"
    else
        info "Command exitted with ${status}"
    fi

    if [ "${SANITIZE}" == "ON" ]
    then
        if [ -f /tmp/asan.log* ]
        then
            cat /tmp/asan.log*
        fi
    fi

    if [ -n ${critical} ]
    then
        if [ ${status} -ne 0 ]
        then
            exit ${status}
        fi
    fi

    return ${status}
}

[ -z "${1}" ] && die "No command given"

if [ "$(readlink -f "${BASE_DIR}")" == "$(readlink -f .)" ]
then
    BUILD_DIR="build"
fi

declare -a ARGS
COMMAND=
NINJA_FLAGS=
OPTIMIZE="ON"
LTO="OFF"
COVERAGE="OFF"
SANITIZE="OFF"
PROFILE="OFF"
BUILD_TESTS="OFF"

read_cache
read_flags "${@}"
regenerate_cmake

run_command -e "src/cpu/isa/generate.py"

case "${COMMAND}" in
    build)
        shift
        build_target vgb
        ;;
    run)
        shift
        build_target "vgb" "${NINJA_FLAGS}"
        run_command -t "${BUILD_DIR}/vgb" "${ARGS[@]}"
        ;;
    test)
        shift
        if [ "${COVERAGE}" == "ON" ]
        then
            build_target "test" "${NINJA_FLAGS}"
            ninja tests-cov-html
        else
            build_target "test" "${NINJA_FLAGS}"
            run_command -t "${BUILD_DIR}/test/test" "${ARGS}"
        fi
        ;;
    *)
        die "Missing command: ${COMMAND}"
        ;;
esac
