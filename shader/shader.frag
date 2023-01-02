#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 Texcoord;

layout(set = 0, binding = 1) uniform UniformBuffer {
    vec3 color;
} ubo;

layout(set = 0, binding = 2) uniform sampler2D Sampler;

void main() {
    outColor = vec4(ubo.color, 1.0) * texture(Sampler, Texcoord);
}
