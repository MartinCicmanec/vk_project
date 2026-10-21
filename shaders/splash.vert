#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"

// One camera-facing quad (2 triangles, 6 vertices) per particle, read straight from the particle buffer - no vertex buffer

struct SplashParticle {
    vec4 positionLife;
    vec4 velocitySize;
};
layout(std430, binding = 4) readonly buffer SplashParticles {
    SplashParticle particles[];
};

layout(location = 0) out vec2 outCorner;            // -1 .. 1 across the quad
layout(location = 1) out vec3 outWorldPosition;
layout(location = 2) out float outOpacity;

const vec2 CORNERS[6] = vec2[]( vec2( -1.0, -1.0 ), vec2( 1.0, -1.0 ), vec2( 1.0, 1.0 ),
                                vec2( -1.0, -1.0 ), vec2( 1.0, 1.0 ), vec2( -1.0, 1.0 ) );

void main() {
    SplashParticle particle = particles[gl_VertexIndex / 6];
    vec2 corner = CORNERS[gl_VertexIndex % 6];
    outCorner = corner;
    outWorldPosition = particle.positionLife.xyz;
    // Fades out in the last half second of its life
    outOpacity = clamp( particle.positionLife.w * 2.0, 0.0, 1.0 );

    if( particle.positionLife.w <= 0.0 ) {
        // Dead: all 6 vertices at the same point outside the screen - the triangles have no area and are dropped
        gl_Position = vec4( 2.0, 2.0, 2.0, 1.0 );
        return;
    }

    // The corner offset is added in view space, so the quad always faces the camera
    vec4 view_position = ubo.view * vec4( particle.positionLife.xyz, 1.0 );
    view_position.xy += corner * particle.velocitySize.w;
    gl_Position = ubo.projection * view_position;
}
