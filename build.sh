#!/bin/bash
set -e

mkdir -p src/spvs

echo "Compiling shaders..."
glslc src/shaders/shader.vert -o src/spvs/vert.spv
glslc src/shaders/shader.frag -o src/spvs/frag.spv

echo "Configuring CMake..."
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release

echo "Building..."
cmake --build build

echo "Running..."
./build/vk_app
