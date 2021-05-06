#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec3 nrm;
layout(location=2) in float fltcol;
layout(location=3) in vec2 uv;

layout(location=0) uniform mat4 proj_mat;
layout(location=1) uniform mat4 view_mat;
layout(location=2) uniform mat4 model_mat;
layout(location=3) uniform mat3 normal_mat;
layout(location=4) uniform float splat_radius;
layout(location=5) uniform ivec4 viewport;

layout(location=0) out vec3 c_eye;
layout(location=1) out vec3 c_eye_nrm;
layout(location=2) out vec2 c_px;
layout(location=3) out vec4 rgba;
layout(location=4) out vec2 tex_uv;

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
    vec4 eye = view_mat * model_mat * vec4(pos, 1);
    c_eye = eye.xyz;
    c_eye_nrm = normal_mat * nrm;

    vec4 ndc = proj_mat * eye;
    gl_Position = ndc;

    vec2 res = vec2(viewport.zw);

    c_px = 0.5 * (ndc.xy / ndc.w + 1.0) * res;

    //float w = splat_radius * (proj_mat[1][1] / abs(eye.z));
    //gl_PointSize = 5.0; //max(w * viewport.z, w * viewport.w) + 1.0;
    float shortening = abs(proj_mat[1][1] / eye.z);
    vec2 footprint = shortening * splat_radius * cos(c_eye_nrm.xy);
    footprint = footprint * res;
    gl_PointSize = max(footprint.x, footprint.y) + 1.0;

    rgba = unpack_rgba(fltcol);
    tex_uv = uv;
}
