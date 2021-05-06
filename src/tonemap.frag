#version 450

#include <util.comp>
#include <tonemap.comp>

layout(location=0) uniform sampler2D linear_col;
layout(location=1) uniform sampler3D tonemap_lut;
layout(location=2) uniform bool use_gamma_only;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 color;

void main() {
    ivec2 center = ivec2(gl_FragCoord.xy);

    vec3 lin_col = texelFetch(linear_col, center, 0).rgb;

    vec3 ldr_col;
    if (use_gamma_only) {
        ldr_col = gamma_map(lin_col);
    } else {
        vec3 log_col = lin_to_log( lin_col + log_to_lin(vec3(0.0)));
        ldr_col = texture(tonemap_lut, log_col).rgb;
    }
    /* float luminance = dot(ldr_col, vec3(0.299f, 0.587f, 0.114f)); */
    float luminance = 1.0;
    color = saturate(vec4(ldr_col, luminance));
}
