#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec3 nrm;
layout(location=2) in float fltcol;
layout(location=3) in vec2 uv;

layout(location=0) uniform mat4 proj_mat;
layout(location=1) uniform mat4 view_mat;
layout(location=2) uniform mat4 model_mat;
layout(location=3) uniform mat3 normal_mat;

/*layout(location=0) out vec3 global_pos;*/
layout(location=0) out vec3 global_nrm;
layout(location=1) out vec4 rgba;
layout(location=2) out vec2 tex_uv;

vec4
unpack_rgba(float fltcol) {
    uint pack = floatBitsToUint(fltcol);
    vec4 col;
    col.r = float((pack >>  0) & 255) / 255.0;
    col.g = float((pack >>  8) & 255) / 255.0;
    col.b = float((pack >> 16) & 255) / 255.0;
    col.a = float((pack >> 24) & 255) / 255.0;
    return col;
}

void main() {
    gl_Position = proj_mat * view_mat * model_mat * vec4(pos, 1);
    /*global_pos = (view_mat * vec4(pos, 1)).xyz;*/
    global_nrm = normal_mat * nrm;
    rgba = unpack_rgba(fltcol);
    tex_uv = uv;
}
