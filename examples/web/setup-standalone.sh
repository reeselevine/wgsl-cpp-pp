#!/bin/bash

# Setup script to prepare the web example as a standalone static site
# This copies the necessary dist files into the web directory

set -e

echo "Setting up standalone web example..."

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/../.." && pwd )"

# Create dist directory in web example
mkdir -p "$SCRIPT_DIR/dist"

# Copy the built files
echo "Copying dist files..."
cp "$PROJECT_ROOT/wasm/dist/index.js" "$SCRIPT_DIR/dist/"
cp "$PROJECT_ROOT/wasm/dist/index.d.ts" "$SCRIPT_DIR/dist/"
cp "$PROJECT_ROOT/wasm/dist/pre-wgsl.mjs" "$SCRIPT_DIR/dist/"
cp "$PROJECT_ROOT/wasm/dist/pre-wgsl.wasm" "$SCRIPT_DIR/dist/"

echo "✓ Setup complete!"
echo ""
echo "The web example is now standalone and can be deployed."
echo "To test locally, run from this directory:"
echo "  http-server -p 8080"
echo "Then open http://localhost:8080/"
