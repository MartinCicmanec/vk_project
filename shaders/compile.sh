#!/bin/bash

# Alternatively use libshaderc from VulkanSDK to compile shader from code
# https://github.com/google/shaderc

glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv