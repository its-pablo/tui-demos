#!/bin/bash

# Get location of this script
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# Run the build script
bash -c "$SCRIPT_DIR/build.sh"

# Run the project passing any arguments
$SCRIPT_DIR/build/gardener "$@"
