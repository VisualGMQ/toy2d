#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO {
    vec3 color;
} ubo;

void main() {
    outColor = vec4(ubo.color, 1);
}
