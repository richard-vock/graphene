#version 460

#include <util.comp>
#include <splats.comp>
#include <tonemap.comp>

layout(location=0) in vec3 c_eye;
layout(location=1) in vec3 c_eye_nrm;
layout(location=2) in vec2 c_px_delta;
layout(location=3) in vec4 rgba;
layout(location=4) in vec2 tex_uv;

layout(location=0) uniform ivec4 viewport;
layout(location=1) uniform mat4 proj_mat;
layout(location=2) uniform float splat_radius;
layout(location=3) uniform bool use_texture;
layout(location=4) uniform sampler2D tex;

layout(location=0) out vec4 color;

void main() {
    vec2 px = gl_FragCoord.xy + c_px_delta;
    vec3 q = splat_projection(px,
                              vec2(viewport.zw),
                              c_eye,
                              c_eye_nrm,
                              proj_mat);
    float d = length(q - c_eye);
    if (d > splat_radius) {
        discard;
    }

    float alpha = 1.0;
    vec4 diff_col = rgba;
    if (use_texture) {
        diff_col = texture(tex, tex_uv).rgba;
    }
    color = vec4(inv_gamma_map(diff_col.rgb), alpha);

    gl_FragDepth = 0.5 * (1.0 - proj_mat[2][2] - proj_mat[3][2]/q.z);
}
