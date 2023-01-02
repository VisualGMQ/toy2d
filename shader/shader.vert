#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec2 outTexcoord;

layout(set = 0, binding = 0) uniform UniformBuffer {
    mat4 project;
    mat4 view;
} ubo;

layout(push_constant) uniform PushConstant {
    mat4 model;
} pc;

void main() {
    gl_Position = ubo.project * ubo.view * pc.model * vec4(inPosition, 0.0, 1.0);
    outTexcoord = inTexcoord;
}
