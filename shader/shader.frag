#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 Texcoord;

layout(set = 1, binding = 0) uniform sampler2D Sampler;

layout(push_constant) uniform PushConstant {
    layout(offset = 64) vec3 color;
} pc;

void main() {
    outColor = vec4(pc.color, 1.0) * texture(Sampler, Texcoord);
}
