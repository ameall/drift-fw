#!/bin/bash
set -e

EXIT_SUCCESS=0
EXIT_FAILURE=1

RED="\033[31m"
GREEN="\033[32m"
RESET="\033[0m"

ROOT_DIR="lidar-app"
BUILD_DIR="build"

check_run_directory() {
    local build_run_dir="$1"
    local current_dir="$(basename $PWD)"

    if [[ "$current_dir" != "$build_run_dir" ]]; then
        echo "Attempting to build lidar-app from an incorrect directory"
        echo "Navigate to lidar-app and then try again"
        return "$EXIT_FAILURE"
    fi
}

if ! check_run_directory "$ROOT_DIR"; then
    echo -e "${RED}BUILD FAILED"
    exit "$EXIT_FAILURE"
fi

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi
cd "$BUILD_DIR"

if ! cmake ..; then
    echo -e "${RED}BUILD FAILED"
    exit "$EXIT_FAILURE"
fi

if ! cmake --build .; then
    echo -e "${RED}BUILD FAILED"
    exit "$EXIT_FAILURE"
fi

echo -e "${GREEN}BUILD SUCCEEDED${RESET}"

exit "$EXIT_SUCCESS"
