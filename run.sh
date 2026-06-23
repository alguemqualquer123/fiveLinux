#!/bin/bash
# FiveMLinuxSDK - Quick Launch Script
# Build and run in one step

set -e

echo "Building FiveMLinuxSDK..."
./build.sh Release

echo ""
echo "Running fivem-linux..."
./build/src/cli/fivem-linux "$@"
