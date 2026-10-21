#!/bin/sh
# Compiles the GLSL shaders in this directory to SPIR-V (requires glslc on PATH)
cd "$(dirname "$0")" || exit 1
glslc shader.vert -o vert.spv || exit 1
glslc shader.frag -o frag.spv || exit 1
glslc particle.comp -o particle_comp.spv || exit 1
glslc particle.vert -o particle_vert.spv || exit 1
glslc particle.frag -o particle_frag.spv || exit 1
glslc water.vert -o water_vert.spv || exit 1
glslc water.frag -o water_frag.spv || exit 1
glslc sky.vert -o sky_vert.spv || exit 1
glslc sky.frag -o sky_frag.spv || exit 1
glslc object.vert -o object_vert.spv || exit 1
glslc object.frag -o object_frag.spv || exit 1
glslc ripple.comp -o ripple_comp.spv || exit 1
glslc splash.comp -o splash_comp.spv || exit 1
glslc splash.vert -o splash_vert.spv || exit 1
glslc splash.frag -o splash_frag.spv || exit 1
