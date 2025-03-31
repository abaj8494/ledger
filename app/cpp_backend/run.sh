#!/bin/bash
set -e

# Check if executable exists
if [ ! -f "build/ledger_api" ]; then
    echo "Executable not found. Building first..."
    ./build.sh
fi

# Run the API server
cd build
./ledger_api ../ledger_api_config.json 