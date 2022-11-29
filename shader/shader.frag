#version 450

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform UniformBuffer {
    vec3 color;
} ubo;

void main() {
    outColor = vec4(ubo.color, 1.0);
}