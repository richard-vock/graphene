#version 460

layout(location=0) in vec2 uv;
layout(location=0) out vec4 color;

layout(location=0) uniform sampler2D tex;
layout(location=1) uniform sampler2D red;

void main() {
    vec3 rgb = texture(tex, uv).rgb;
    float red = texture(red, uv).r;
    float lum = 0.2126*rgb.r + 0.7152*rgb.g + 0.0722*rgb.b;
    color = vec4(red, lum, lum, 1.0);
}
