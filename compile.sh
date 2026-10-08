#!/bin/sh
# Clean build: deletes the build folder, configures and builds the project, then compiles the shaders
set -e
cd "$(dirname "$0")"

rm -rf build
cmake -S . -B build "$@"
cmake --build build
./shaders/compile.sh

echo "Build finished. Run ./bin/main from this folder."
