#version 460

#include <util.comp>

layout(location=0) uniform sampler2D gbuffer;
layout(location=1) uniform vec4 background_inner;
layout(location=2) uniform vec4 background_outer;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 color;

void main() {
    vec4 rgbw = texture(gbuffer, uv).rgba;
    float weight = rgbw.w;
    if (weight < 0.0001) {
        // background pixel
        color = background_gradient(background_inner, background_outer, uv);
    } else {
        color = vec4(rgbw.rgb / weight, 1.0);
    }
}

