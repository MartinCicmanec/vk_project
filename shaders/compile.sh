#!/bin/sh
# Compiles the GLSL shaders in this directory to SPIR-V (requires glslc on PATH)
cd "$(dirname "$0")" || exit 1
glslc shader.vert -o vert.spv || exit 1
glslc shader.frag -o frag.spv || exit 1
