#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 0) out vec3 outColor;

layout (binding = 0) uniform UniformBufferObject {
    mat4 project;
    mat4 view;
    mat4 model;
} ubo;

void main() {
    gl_Position = ubo.project * ubo.view * ubo.model * vec4(inPos, 0, 1);
    outColor = inColor;
}

