#version 460

#include <util.comp>
#include <splats.comp>
#include <tonemap.comp>

layout(location=0) in vec3 c_eye;
layout(location=1) in vec3 c_eye_nrm;
layout(location=2) in vec4 rgba;
layout(location=3) in vec2 tex_uv;

layout(location=0) uniform bool use_texture;
layout(location=1) uniform sampler2D tex;

layout(location=0) out vec4 color;

void main() {
    float alpha = 1.0;
    vec4 diff_col = rgba;
    if (use_texture) {
        diff_col = texture(tex, tex_uv).rgba;
    }
    color = diff_col;
    /* color = vec4(inv_gamma_map(diff_col.rgb), alpha); */
}
