#version 450

// One triangle that covers the whole screen, without a vertex buffer: vertices (-1, -1), (3, -1), (-1, 3)
layout(location = 0) out vec2 outNdc;

void main() {
    vec2 uv = vec2( (gl_VertexIndex << 1) & 2, gl_VertexIndex & 2 );
    outNdc = uv * 2.0 - 1.0;
    // Depth 1.0 is the far plane: drawn after the water with LESS_OR_EQUAL, so only pixels without water are shaded
    gl_Position = vec4( outNdc, 1.0, 1.0 );
}
