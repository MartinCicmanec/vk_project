#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"
#include "sky.glsl"

layout(location = 0) in vec2 inNdc;

layout(location = 0) out vec4 outColor;

void main() {
    // The point of this pixel on the far plane, back in world space - the view direction goes from the camera to it
    vec4 far_point = ubo.inverseViewProjection * vec4( inNdc, 1.0, 1.0 );
    vec3 direction = normalize( far_point.xyz / far_point.w - ubo.cameraPositionAndTime.xyz );
    outColor = vec4( SkyColor( direction, ubo.sunDirection.xyz ), 1.0 );
}
