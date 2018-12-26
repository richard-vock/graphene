#version 460

layout(location=0) uniform sampler2D tex;

layout(location=0) in vec2 fuv;
layout(location=0) out vec4 color;
layout(location=1) out vec4 red;

void main() {
    vec4 fcol = texture(tex, fuv).rgba;
    color = fcol;
    red = vec4(fcol.r, 0.0, 0.0, 1.0);
}
