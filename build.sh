#!/bin/bash

# Get location of this script
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# Create build directory in the script's directory
BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Run CMake to configure the project
cmake "$SCRIPT_DIR"

# Build the project
make "$@"
