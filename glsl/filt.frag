#version 460

layout(location=0) uniform sampler2D tex;
layout(location=1) uniform int width;
layout(location=2) uniform int height;
layout(location=3) uniform bool up;

layout(location=0) in vec2 uv;

layout(location=0) out float filtered;

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
    vec2 shift = vec2(0.5 / width, 0.5 / height);
    float dir = up ? 1.0 : -1.0;
    filtered = texture(tex, uv + dir * shift).r;
}
