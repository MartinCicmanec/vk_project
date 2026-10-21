#version 450

// The particle buffer is used as the vertex buffer - velocity is skipped (see Particle::GetAttributeDescriptions)
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    // Sizes other than 1.0 require the largePoints device feature
    gl_PointSize = 14.0;
    gl_Position = vec4(inPosition.xy, 1.0, 1.0);
    fragColor = inColor.rgb;
}
