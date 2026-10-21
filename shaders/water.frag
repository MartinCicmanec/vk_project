#version 450
#extension GL_GOOGLE_include_directive : require

#include "ocean.glsl"
#include "sky.glsl"

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec2 inGridPosition;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 camera_position = ubo.cameraPositionAndTime.xyz;
    vec3 sun_direction = ubo.sunDirection.xyz;

    // Size of this pixel on the water - waves smaller than a few pixels are faded out instead of shimmering
    vec2 footprint = fwidth( inGridPosition );
    vec3 normal = GerstnerNormal( inGridPosition, ubo.cameraPositionAndTime.w, max( footprint.x, footprint.y ) );

    // Ripples tilt the normal by their slope (central differences, one ripple cell apart). Adding the slope to the
    // Gerstner normal is an approximation that holds for the small slopes of both.
    float ripple_cell = 2.0 * ubo.rippleArea.x / ubo.rippleArea.y;
    vec2 ripple_slope = vec2( RippleHeight( inWorldPosition.xz + vec2( ripple_cell, 0.0 ) ) - RippleHeight( inWorldPosition.xz - vec2( ripple_cell, 0.0 ) ),
                              RippleHeight( inWorldPosition.xz + vec2( 0.0, ripple_cell ) ) - RippleHeight( inWorldPosition.xz - vec2( 0.0, ripple_cell ) ) ) / (2.0 * ripple_cell);
    normal = normalize( vec3( normal.x - ripple_slope.x * normal.y, normal.y, normal.z - ripple_slope.y * normal.y ) );

    vec3 to_camera = camera_position - inWorldPosition;
    float distance_to_camera = length( to_camera );
    vec3 view_direction = to_camera / distance_to_camera;

    // At grazing angles an interpolated normal can face away from the camera - clamp instead of going black
    float n_dot_v = max( dot( normal, view_direction ), 0.0 );

    // Fresnel (Schlick, water F0 = 0.02): almost no reflection looking down, almost a mirror at grazing angles
    float fresnel = 0.02 + 0.98 * pow( 1.0 - n_dot_v, 5.0 );

    // Reflected sky. A reflection pointing below the horizon would hit other waves - the sky above is a cheap stand-in.
    vec3 reflected = reflect( -view_direction, normal );
    reflected.y = abs( reflected.y );
    vec3 reflection = SkyColor( reflected, sun_direction );

    // Light scattered inside the water: deep blue, more turquoise on the crests (thinner water lit from behind)
    vec3 deep_color    = vec3( 0.005, 0.03, 0.07 );
    vec3 scatter_color = vec3( 0.0, 0.16, 0.14 );
    float crest = clamp( inWorldPosition.y * 0.6 + 0.3, 0.0, 1.0 );
    float sun_facing = 0.5 + 0.5 * max( dot( normal, sun_direction ), 0.0 );
    vec3 water_color = deep_color + scatter_color * crest * sun_facing;

    // Sun glints
    float specular = pow( max( dot( reflected, sun_direction ), 0.0 ), 720.0 ) * 6.0;

    vec3 color = mix( water_color, reflection, fresnel ) + SUN_COLOR * specular;

    // Haze towards the horizon color - also hides the end of the water grid
    float haze = 1.0 - exp( -pow( distance_to_camera / 280.0, 2.0 ) );
    vec3 horizon = SkyColor( normalize( vec3( -view_direction.x, 0.0, -view_direction.z ) ), sun_direction );
    outColor = vec4( mix( color, horizon, haze ), 1.0 );
}
