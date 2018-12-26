#version 460

layout(location=0) uniform mat4 view_mat;
layout(location=1) uniform bool use_texture;
layout(location=2) uniform bool shade;
layout(location=3) uniform sampler2D tex;

/*layout(location=0) in vec3 global_pos;*/
layout(location=0) in vec3 global_nrm;
layout(location=1) in vec4 rgba;
layout(location=2) in vec2 tex_uv;

layout(location=0) out vec4 color;

void main() {
    vec4 diff_col = rgba;
    if (use_texture) {
        diff_col = texture(tex, tex_uv).rgba;
    }

    if (shade) {
        vec4 light_dir = view_mat * vec4(1, 1, 0, 0);
        diff_col.rgb *= clamp(dot(normalize(light_dir.xyz), global_nrm), 0.0, 1.0);
    }

    color = diff_col;
}
