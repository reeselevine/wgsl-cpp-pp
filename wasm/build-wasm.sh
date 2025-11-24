#!/bin/bash

set -e

echo "Building pre-wgsl to WebAssembly..."

# Check if emcmake is available
if ! command -v emcmake &> /dev/null; then
    echo "Error: emcmake not found. Please install Emscripten SDK."
    exit 1
fi

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

mkdir -p dist

echo "Configuring with Emscripten..."
cd "$BUILD_DIR"
emcmake cmake ..

echo "Building..."
cmake --build .

echo "Build complete! Output files are in dist/"
ls -lh ../dist/pre-wgsl.*
