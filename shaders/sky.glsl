// Procedural sky, shared by sky.frag (background) and water.frag (reflections and haze) - included, not compiled on its own.
// All colors are linear - the swapchain uses an sRGB format, which converts them when writing.

const vec3 SUN_COLOR = vec3( 1.0, 0.85, 0.65 );

// Color of the sky in the given direction (unit vector). Below the horizon the horizon color is used.
vec3 SkyColor( vec3 direction, vec3 sun_direction ) {
    vec3 zenith_color  = vec3( 0.08, 0.25, 0.60 );
    vec3 horizon_color = vec3( 0.60, 0.72, 0.85 );

    float height = max( direction.y, 0.0 );
    vec3 color = mix( horizon_color, zenith_color, sqrt( height ) );

    // Wide glow and a sharp disk around the sun
    float sun_alignment = max( dot( direction, sun_direction ), 0.0 );
    color += SUN_COLOR * (0.25 * pow( sun_alignment, 8.0 ) + 0.4 * pow( sun_alignment, 128.0 ));
    color += SUN_COLOR * 4.0 * smoothstep( 0.9994, 0.9997, sun_alignment );
    return color;
}
