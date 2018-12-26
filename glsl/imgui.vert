#version 460

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

layout (location = 0) out vec2 frag_uv;
layout (location = 1) out vec4 frag_color;

layout (location = 0) uniform mat4 mvp;

void main() {
    frag_uv = uv;
    frag_color = color;
    gl_Position = mvp * vec4(position.xy,0,1);
}
