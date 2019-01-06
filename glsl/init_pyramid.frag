#version 460

layout(location=0) uniform sampler2D depth_tex;
layout(location=1) uniform mat4 proj_mat;
layout(location=2) uniform vec2 near_size;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 pyramid;

vec3
unproject(float depth) {
    // [0,1] to [-1,1]
    float z = 2.0*depth - 1.0;
    // NDC to eye; z in [-near, -far]
    z = proj_mat[3][2] / (proj_mat[2][3] * z - proj_mat[2][2]);
    vec3 eye_dir = vec3((2.0 * uv - 1.0) * near_size, -1.0);
    // we have
    //     pos = t * eye_dir
    // with
    //     t = |z| = -z  (since z < 0)
    return vec3(-z * eye_dir);
}

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
    float depth = texelFetch(depth_tex, center, 0).r;
    pyramid = vec4(unproject(depth), depth);
}
