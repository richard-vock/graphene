#version 460

layout(location=0) uniform sampler2D depth_tex;
layout(location=1) uniform mat4 proj_mat;
layout(location=2) uniform vec2 near_size;
layout(location=3) uniform int width;
layout(location=4) uniform int height;
layout(location=5) uniform float occlusion_threshold;
layout(location=6) uniform uint kernel_size;

layout(location=0) in vec2 uv;

layout(location=0) out vec2 map;

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

float
occlusion(ivec2 center, float depth) {
    vec3 p0 = unproject(depth);
    vec3 v = normalize(-p0);
    int ext = (int(kernel_size) - 1) / 2;
    float sectors[8];
    sectors[0] = 0.0;
    sectors[1] = 0.0;
    sectors[2] = 0.0;
    sectors[3] = 0.0;
    sectors[4] = 0.0;
    sectors[5] = 0.0;
    sectors[6] = 0.0;
    sectors[7] = 0.0;
    for (int x = -ext; x <= ext; ++x) {
        int i = center.x + x;
        if (i < 0 || i >= width) {
            continue;
        }
        bool left = x <= 0;
        bool right = x >= 0;
        for (int y = -ext; y <= ext; ++y) {
            int j = center.y + y;
            if (j < 0 || j >= height || (x == 0 && y == 0)) {
                continue;
            }

            bool bot = y <= 0;
            bool top = y >= 0;

            float ndepth = texelFetch(depth_tex, ivec2(i,j), 0).r;
            if (ndepth >= depth) continue;

            // we have a horizon pixel
            vec3 npos = unproject(ndepth);
            float aperture = dot(normalize(npos-p0), v);

            if (left) {
                if (bot) {
                    if (x >= y) sectors[0] = max(sectors[0], aperture);
                    if (x <= y) sectors[1] = max(sectors[1], aperture);
                }
                if (top) {
                    if ((-x) >= y) sectors[2] = max(sectors[2], aperture);
                    if ((-x) <= y) sectors[3] = max(sectors[3], aperture);
                }
            }

            if (right) {
                if (top) {
                    if (x <= y) sectors[4] = max(sectors[4], aperture);
                    if (x >= y) sectors[5] = max(sectors[5], aperture);
                }
                if (bot) {
                    if (x >= (-y)) sectors[6] = max(sectors[6], aperture);
                    if (x <= (-y)) sectors[7] = max(sectors[7], aperture);
                }
            }
        }
    }

    float occlusion = sectors[0]
                    + sectors[1]
                    + sectors[2]
                    + sectors[3]
                    + sectors[4]
                    + sectors[5]
                    + sectors[6]
                    + sectors[7];
    return occlusion / 8;
}

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
    float depth = texelFetch(depth_tex, center, 0).r;
    map.r = occlusion(center, depth) > occlusion_threshold ? 1.0 : 0.0;
    map.g = occlusion(center, depth) > occlusion_threshold ? depth : 1.0;
}
