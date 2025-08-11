#!/bin/bash

# Get location of this script
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# Change to the build directory
BUILD_DIR="$SCRIPT_DIR/build"
cd "$BUILD_DIR"

# Make the clean target
make clean

# Change to the debug directory
DEBUG_DIR="$SCRIPT_DIR/debug"
cd "$DEBUG_DIR"

# Make the clean target
make clean

# Remove the build and debug directory
cd ..
rm -rf "$BUILD_DIR"
rm -rf "$DEBUG_DIR"
