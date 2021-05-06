#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec2 uv;

layout(location=0) uniform mat4 proj_mat;
layout(location=1) uniform mat4 view_mat;

layout(location=0) out vec2 fuv;

void main() {
    gl_Position = proj_mat * view_mat * vec4(pos, 1);
    fuv = uv;
}
