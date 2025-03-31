#!/bin/bash
set -e

# Change to build directory
mkdir -p build
cd build

# Run CMake
cmake ..

# Build the project
make -j$(nproc)

echo "Build complete! The executable is at build/ledger_api" 