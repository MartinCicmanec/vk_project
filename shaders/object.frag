#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"
#include "sky.glsl"

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize( inNormal );
    vec3 sun_direction = ubo.sunDirection.xyz;

    // Direct sun light, plus light from the sky in the direction the surface faces (dimmed - the sky is bright)
    float sun_light = max( dot( normal, sun_direction ), 0.0 );
    vec3 ambient = 0.35 * SkyColor( normalize( vec3( normal.x, abs( normal.y ), normal.z ) ), sun_direction );
    vec3 color = inColor * (SUN_COLOR * sun_light + ambient);

    // Same haze as the water
    vec3 to_camera = ubo.cameraPositionAndTime.xyz - inWorldPosition;
    float distance_to_camera = length( to_camera );
    vec3 view_direction = to_camera / distance_to_camera;
    float haze = 1.0 - exp( -pow( distance_to_camera / 280.0, 2.0 ) );
    vec3 horizon = SkyColor( normalize( vec3( -view_direction.x, 0.0, -view_direction.z ) ), sun_direction );
    outColor = vec4( mix( color, horizon, haze ), 1.0 );
}
