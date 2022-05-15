#version 410 core

vec2 Vertices[3] = vec2[](
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5),
    vec2(0, 0.5)
);

void main() {
    gl_Position = vec4(Vertices[gl_VertexIndex], 0, 1);
}
