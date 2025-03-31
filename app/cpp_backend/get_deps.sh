#!/bin/bash
set -e

# Create directories
mkdir -p ext

# Download Crow
if [ ! -d "ext/crow" ]; then
    echo "Downloading Crow..."
    git clone https://github.com/CrowCpp/Crow.git ext/crow
fi

# Download nlohmann/json
if [ ! -d "ext/json" ]; then
    echo "Downloading nlohmann/json..."
    git clone https://github.com/nlohmann/json.git ext/json
    
    # Create include directory if it doesn't exist
    mkdir -p include/nlohmann
    
    # Copy the single header file
    cp ext/json/single_include/nlohmann/json.hpp include/nlohmann/
fi

echo "Dependencies downloaded successfully!" 