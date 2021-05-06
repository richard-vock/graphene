#version 460

layout(location=0) uniform mat4 view_mat;
layout(location=1) uniform bool use_texture;
layout(location=2) uniform bool shade;
layout(location=3) uniform sampler2D tex;
layout(location=4) uniform ivec4 viewport;
layout(location=5) uniform mat4 proj_mat_inv;
layout(location=6) uniform float splat_radius;
layout(location=7) uniform mat4 proj_mat;


/*layout(location=0) in vec3 global_pos;*/
layout(location=0) in vec3 c_eye;
layout(location=1) in vec3 c_eye_nrm;
layout(location=2) in vec4 rgba;
layout(location=3) in vec2 tex_uv;

layout(location=0) out vec4 color;

void main() {
    vec4 p_ndc = vec4(2.0 * gl_FragCoord.xy / (viewport.zw) - 1.0, -1.0, 1.0);
    vec4 p_eye = proj_mat_inv * p_ndc;
    vec3 qn = p_eye.xyz / p_eye.w;
    vec3 q = qn * dot(c_eye, c_eye_nrm) / dot(qn, c_eye_nrm);
    float d = length(q - c_eye);

    if (d > splat_radius) {
        discard;
    }

    vec4 diff_col = rgba;
    if (use_texture) {
        diff_col = texture(tex, tex_uv).rgba;
    }

    float depth = -proj_mat[3][2] / q.z - proj_mat[2][2];
    gl_FragDepth = (depth + 1.0) / 2.0;

    if (shade) {
        vec4 light_dir = view_mat * vec4(1, 1, 0, 0);
        diff_col.rgb *= clamp(dot(normalize(light_dir.xyz), c_eye_nrm), 0.0, 1.0);
    }

    color = diff_col;
}
