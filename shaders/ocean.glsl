// Shared by water.vert, water.frag, sky.frag - included, not compiled on its own.
// The uniform block must match struct OceanUniforms in include/Ocean.h.

const int OCEAN_WAVE_COUNT = 8;

struct GerstnerWave {
    vec4 directionWavenumberFrequency;  // xy: direction (unit, in the xz plane), z: wavenumber k = 2 pi / wavelength, w: angular frequency
    vec4 amplitudeSteepnessPhase;       // x: amplitude, y: steepness Q (horizontal pinch), z: phase offset, w: unused
};

layout(binding = 0) uniform OceanUniforms {
    mat4 view;
    mat4 projection;
    mat4 inverseViewProjection;
    vec4 cameraPositionAndTime;         // xyz: camera position, w: time in seconds
    vec4 sunDirection;                  // xyz: unit vector towards the sun
    vec4 rippleArea;                    // x: half size of the ripple field, y: cells per side, z: index of the current height image
    GerstnerWave waves[OCEAN_WAVE_COUNT];
} ubo;

// Ripple field (shaders/ripple.comp) - two height images that take turns, ubo.rippleArea.z says which one is current
layout(binding = 1, r32f) readonly uniform image2D rippleHeight0;
layout(binding = 2, r32f) readonly uniform image2D rippleHeight1;

float RippleTexel( ivec2 cell ) {
    // A branch instead of an image array - indexing with a variable needs an extra device feature
    return (ubo.rippleArea.z < 0.5) ? imageLoad( rippleHeight0, cell ).r : imageLoad( rippleHeight1, cell ).r;
}

// Ripple height at a world position (x, z), interpolated between the 4 nearest cells. Fades out towards the edge of
// the field, so its border isn't visible; 0 outside.
float RippleHeight( vec2 world_position ) {
    float half_size = ubo.rippleArea.x;
    float cell_count = ubo.rippleArea.y;
    vec2 normalized = world_position / half_size;               // -1 .. 1 inside the field
    float edge = max( abs( normalized.x ), abs( normalized.y ) );
    if( edge >= 0.95 ) {
        return 0.0;
    }
    // Cell centers are at (i + 0.5) cells
    vec2 cell_position = (normalized * 0.5 + 0.5) * cell_count - 0.5;
    ivec2 cell = ivec2( floor( cell_position ) );
    vec2 weight = cell_position - vec2( cell );
    ivec2 last = ivec2( int( cell_count ) - 1 );
    float h00 = RippleTexel( clamp( cell, ivec2( 0 ), last ) );
    float h10 = RippleTexel( clamp( cell + ivec2( 1, 0 ), ivec2( 0 ), last ) );
    float h01 = RippleTexel( clamp( cell + ivec2( 0, 1 ), ivec2( 0 ), last ) );
    float h11 = RippleTexel( clamp( cell + ivec2( 1, 1 ), ivec2( 0 ), last ) );
    float height = mix( mix( h00, h10, weight.x ), mix( h01, h11, weight.x ), weight.y );
    return height * (1.0 - smoothstep( 0.75, 0.95, edge ));
}

// Waves shorter than a few samples (grid cells or pixels) can't be represented - they would alias into noise or shimmer.
// They are faded out between 2 and 4 samples per wavelength.
float WaveFade( float wavenumber, float sample_size ) {
    float samples_per_wavelength = 6.2831853 / (wavenumber * max( sample_size, 1e-4 ));
    return smoothstep( 2.0, 4.0, samples_per_wavelength );
}

// Gerstner waves (GPU Gems 1, chapter 1): the grid point (x, z) moves in a circle - up and down, and towards the crests,
// which makes them sharp and the troughs wide. Returns the world position of the grid point.
vec3 GerstnerPosition( vec2 grid_position, float time, float sample_size ) {
    vec3 position = vec3( grid_position.x, 0.0, grid_position.y );
    for( int i = 0; i < OCEAN_WAVE_COUNT; ++i ) {
        vec2  direction = ubo.waves[i].directionWavenumberFrequency.xy;
        float k         = ubo.waves[i].directionWavenumberFrequency.z;
        float omega     = ubo.waves[i].directionWavenumberFrequency.w;
        float amplitude = ubo.waves[i].amplitudeSteepnessPhase.x * WaveFade( k, sample_size );
        float steepness = ubo.waves[i].amplitudeSteepnessPhase.y;
        float phase     = ubo.waves[i].amplitudeSteepnessPhase.z;

        float theta = k * dot( direction, grid_position ) - omega * time + phase;
        position.xz += steepness * amplitude * direction * cos( theta );
        position.y  += amplitude * sin( theta );
    }
    return position;
}

// Exact normal of the Gerstner surface: the cross product of the partial derivatives of GerstnerPosition by x and z
vec3 GerstnerNormal( vec2 grid_position, float time, float sample_size ) {
    vec3 tangent_x = vec3( 1.0, 0.0, 0.0 );
    vec3 tangent_z = vec3( 0.0, 0.0, 1.0 );
    for( int i = 0; i < OCEAN_WAVE_COUNT; ++i ) {
        vec2  direction = ubo.waves[i].directionWavenumberFrequency.xy;
        float k         = ubo.waves[i].directionWavenumberFrequency.z;
        float omega     = ubo.waves[i].directionWavenumberFrequency.w;
        float amplitude = ubo.waves[i].amplitudeSteepnessPhase.x * WaveFade( k, sample_size );
        float steepness = ubo.waves[i].amplitudeSteepnessPhase.y;
        float phase     = ubo.waves[i].amplitudeSteepnessPhase.z;

        float theta = k * dot( direction, grid_position ) - omega * time + phase;
        float ka_sin = k * amplitude * sin( theta );
        float ka_cos = k * amplitude * cos( theta );
        tangent_x += vec3( -steepness * direction.x * direction.x * ka_sin, direction.x * ka_cos, -steepness * direction.x * direction.y * ka_sin );
        tangent_z += vec3( -steepness * direction.x * direction.y * ka_sin, direction.y * ka_cos, -steepness * direction.y * direction.y * ka_sin );
    }
    return normalize( cross( tangent_z, tangent_x ) );
}
