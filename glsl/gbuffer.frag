#version 460

layout(location=0) uniform sampler2D depth_map;
layout(location=1) uniform vec2 nf;
layout(location=2) uniform int width;
layout(location=3) uniform int height;
layout(location=4) uniform vec2 near_size;
layout(location=5) uniform mat3 inv_view_mat;

layout(location=0) in vec2 uv;

layout(location=0) out vec4 gbuffer;

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

const float[] scharr_x = {
    -0.09375f,  0.0f,  0.09375f,
    -0.3125f,   0.0f,  0.3125f,
    -0.09375f,  0.0f,  0.09375f,
};

const float[] scharr_y = {
    -0.09375f, -0.3125f, -0.09375f,
     0.0f,      0.0f,     0.0f,
     0.09375f,  0.3125f,  0.09375f,
};

vec4
color_map(float value) {
    float i = value * 8;
    int lower = int(floor(i));
    int upper = int(ceil(i));
    float interp = i - float(lower);
    return vec4((1.0 - interp) * map_colors[lower] + interp * map_colors[upper], 1.0);
}

vec3
unproject(ivec2 coords, float depth) {
    // uv coordinates
    vec2 xy = vec2(float(coords.x) / float(width-1), float(coords.y) / float(height-1));
    // rescale to [near, far]
    float z = depth * (nf.y-nf.x) + nf.x;
    vec3 eye_dir = vec3((2.0 * xy - 1.0) * near_size, -1.0);
    return z * eye_dir;
}

vec3
derivative_filter(ivec2 center) {
    if (center.x == 0 || center.y == 0 || center.x == (width - 1) || center.y == (height - 1)) {
        return vec3(0.0);
    }

    bool has_neighbors = true;
    vec3 dx = vec3(0.0);
    vec3 dy = vec3(0.0);

    for (int x = -1; x <= 1; ++x) {
        if (!has_neighbors) break;
        int i = center.x + x;
        for (int y = -1; y <= 1; ++y) {
            int j = center.y + y;

            float depth = texelFetch(depth_map, ivec2(i, j), 0).r;
            if (depth > 0.9999) {
                has_neighbors = false;
                break;
            }

            vec3 v = unproject(ivec2(i,j), depth);

            dx += v * scharr_x[(y+1)*3+x+1];
            dy += v * scharr_y[(y+1)*3+x+1];
        }
    }

    return has_neighbors ? normalize(cross(dx, dy)) : vec3(0.0);
}

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));

    vec3 nrm = derivative_filter(center);

    if (length(nrm) < 0.5) discard;
    gbuffer = vec4(inv_view_mat * nrm, 1.0);
}
