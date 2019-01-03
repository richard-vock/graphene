#version 460

/*layout(location=0) uniform sampler2D depth_tex;*/
layout(location=0) uniform sampler2D tex;
layout(location=1) uniform mat4 proj_mat;
layout(location=2) uniform float near;
layout(location=3) uniform float far;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 color;

const vec3[] map_colors = {
    vec3(1.00000, 1.00000, 0.80000),
    vec3(1.00000, 0.92941, 0.62745),
    vec3(0.99608, 0.85098, 0.46275),
    vec3(0.99608, 0.69804, 0.29804),
    vec3(0.99216, 0.55294, 0.23529),
    vec3(0.98824, 0.30588, 0.16471),
    vec3(0.89020, 0.10196, 0.10980),
    vec3(0.74118, 0.00000, 0.14902),
    vec3(0.50196, 0.00000, 0.14902)
};

vec4
color_map(float value) {
    float i = value * 8;
    int lower = int(floor(i));
    int upper = int(ceil(i));
    float interp = i - float(lower);
    return vec4((1.0 - interp) * map_colors[lower] + interp * map_colors[upper], 1.0);
}

float
linearize_depth(float depth) {
    // [0,1] to [-1,1]
    float z = 2.0*depth - 1.0;
    // NDC to eye
    z = proj_mat[3][2] / (proj_mat[2][3] * z - proj_mat[2][2]);
    // z is now in [-near, -far], negate and map to [0, 1]
    z = (-z - near) / (far-near);
    return z;
}

void main() {
    /*float depth = texture(depth_tex, uv).r;*/
    /*color = depth == 1.0 ? vec4(0.0, 0.0, 0.0, 1.0) : color_map(linearize_depth(depth));*/
    float depth = texture(tex, uv).g;
    if (depth >= 1.0) {
        discard;
    }
    color = color_map(linearize_depth(depth));
}
