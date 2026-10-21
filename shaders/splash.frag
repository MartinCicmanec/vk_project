#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"
#include "sky.glsl"

layout(location = 0) in vec2 inCorner;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in float inOpacity;

layout(location = 0) out vec4 outColor;

void main() {
    // Round and soft: opaque in the center, transparent at the edge, nothing outside the circle
    float radius_squared = dot( inCorner, inCorner );
    if( radius_squared >= 1.0 ) {
        discard;
    }
    float alpha = (1.0 - radius_squared) * 0.5 * inOpacity;

    // White spray: water drops scatter light in all directions, so it stays bright from every side - mostly white,
    // a little sun color, a little sky
    vec3 sun_direction = ubo.sunDirection.xyz;
    vec3 color = 0.75 * vec3( 0.95, 0.97, 1.0 ) + 0.15 * SUN_COLOR + 0.1 * SkyColor( vec3( 0.0, 1.0, 0.0 ), sun_direction );

    // Same haze as the water and the boxes
    vec3 to_camera = ubo.cameraPositionAndTime.xyz - inWorldPosition;
    float distance_to_camera = length( to_camera );
    vec3 view_direction = to_camera / distance_to_camera;
    float haze = 1.0 - exp( -pow( distance_to_camera / 280.0, 2.0 ) );
    vec3 horizon = SkyColor( normalize( vec3( -view_direction.x, 0.0, -view_direction.z ) ), sun_direction );
    outColor = vec4( mix( color, horizon, haze ), alpha );
}
