#version 460

layout(location=0) uniform sampler2D pyramid;
layout(location=1) uniform int width;
layout(location=2) uniform int height;
layout(location=3) uniform int level;
layout(location=4) uniform mat4 proj_mat;
layout(location=5) uniform vec2 near_size;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 out_pyramid;

vec4
sample_pyramid(ivec2 px) {
    vec4 xyz_d = vec4(0.0, 0.0, 0.0, 1.0);
    if (px.x <= (width-1) && px.y <= (height-1)) {
        xyz_d = texelFetch(pyramid, px, level-1);
    }
    return xyz_d;
}

float
sample_depth(ivec2 px) {
    float depth = 1.0;;
    return depth;
}

void main() {
    if (level == 0) {
        ivec2 px = ivec2(gl_FragCoord.xy);
        float depth = 1.0;
        if (px.x <= (width-1) && px.y <= (height-1)) {
            depth = texelFetch(pyramid, px, 0).r;
        }
        float z = 2.0*depth - 1.0;
        z = proj_mat[3][2] / (proj_mat[2][3] * z - proj_mat[2][2]);
        vec3 eye_dir = vec3((2.0 * uv - 1.0) * near_size, -1.0);
        out_pyramid.xyz = -z * eye_dir;
        out_pyramid.w = depth;
        return;
    }

    ivec2 px = 2 * ivec2(gl_FragCoord.xy);
    vec4 xyz_d = sample_pyramid(px);
    vec4 nn = sample_pyramid(px + ivec2(1,0));
    if (nn.w < xyz_d.w) {
        xyz_d = nn;
    }
    nn = sample_pyramid(px + ivec2(0,1));
    if (nn.w < xyz_d.w) {
        xyz_d = nn;
    }
    nn = sample_pyramid(px + ivec2(1,1));
    if (nn.w < xyz_d.w) {
        xyz_d = nn;
    }
    out_pyramid = xyz_d;
}
