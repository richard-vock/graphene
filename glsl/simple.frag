#version 460

layout(location=0) in vec4 fcol;
layout(location=0) out vec4 color;
layout(location=1) out vec4 red;

void main() {
    color = fcol;
    red = vec4(fcol.r, 0.0, 0.0, 1.0);
}
