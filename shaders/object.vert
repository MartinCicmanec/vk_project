#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"

// Must match struct BoxVertex and BoxInstance in include/BoxMesh.h
layout(location = 0) in vec3 inPosition;        // unit box, -1 .. 1
layout(location = 1) in vec3 inNormal;
layout(location = 2) in mat4 inTransform;       // per instance - rotation and translation (locations 2 to 5)
layout(location = 6) in vec4 inHalfExtents;     // per instance
layout(location = 7) in vec4 inColor;           // per instance

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;

void main() {
    // Scaled in local space, before the rotation - the normals of a box don't change with the scale
    vec4 world_position = inTransform * vec4( inPosition * inHalfExtents.xyz, 1.0 );
    outWorldPosition = world_position.xyz;
    outNormal = mat3( inTransform ) * inNormal;
    outColor = inColor.rgb;
    gl_Position = ubo.projection * ubo.view * world_position;
}
