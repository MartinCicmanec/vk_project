#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"

// Must match struct OceanGridVertex in include/Ocean.h
layout(location = 0) in vec2  inGridPosition;   // undisplaced position in the xz plane
layout(location = 1) in float inCellSize;       // distance to the neighboring grid points

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec2 outGridPosition;

void main() {
    // Only the position is computed here - the normal is computed per pixel in water.frag, where the grid is too coarse
    // for the small waves (they are faded out here, but still shape the lighting)
    vec3 world_position = GerstnerPosition( inGridPosition, ubo.cameraPositionAndTime.w, inCellSize );
    // Ripples and wakes of the floating objects on top
    world_position.y += RippleHeight( world_position.xz );

    outWorldPosition = world_position;
    outGridPosition = inGridPosition;
    gl_Position = ubo.projection * ubo.view * vec4( world_position, 1.0 );
}
