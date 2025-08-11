#!/bin/bash

# Get location of this script
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# Change to the build directory
BUILD_DIR="$SCRIPT_DIR/build"
cd "$BUILD_DIR"

# Build the project
make clean

# Remove the build directory
cd ..
rm -rf "$BUILD_DIR"
