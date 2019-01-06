#version 460

layout(location=0) uniform sampler2D pyramid;
layout(location=1) uniform int width;
layout(location=2) uniform int height;
layout(location=3) uniform float occlusion_threshold;
layout(location=4) uniform vec2 nf;
layout(location=5) uniform int max_level;
layout(location=6) uniform float lod_factor;

layout(location=0) in vec2 uv;

layout(location=0) out vec2 map;

float
lod_occlusion(ivec2 center) {
    vec4 xyz_d = texelFetch(pyramid, center, 0);
    vec3 v = normalize(-xyz_d.xyz);

    int level_bound = clamp(int(floor(log(lod_factor / (-xyz_d.z)) / log(2))), 0, max_level);

    ivec2 offsets[8] = {
        ivec2(-1,-1),
        ivec2(-1, 0),
        ivec2(-1, 1),
        ivec2( 0, 1),
        ivec2( 1, 1),
        ivec2( 1, 0),
        ivec2( 1,-1),
        ivec2( 0,-1)
    };

    float sectors[8];
    sectors[0] = 0.0;
    sectors[1] = 0.0;
    sectors[2] = 0.0;
    sectors[3] = 0.0;
    sectors[4] = 0.0;
    sectors[5] = 0.0;
    sectors[6] = 0.0;
    sectors[7] = 0.0;

    int w = width;
    int h = height;
    ivec2 c = center;
    for(int level = 0; level <= level_bound; ++level) {
        for(int i=0; i<8; ++i) {
            ivec2 px = c + offsets[i];
            if (px.x >= 0 && px.y >= 0 && px.x <= (w-1) && px.y <= (h-1)) {
                vec4 n_xyz_d = texelFetch(pyramid, px, level);
                if (n_xyz_d.w < xyz_d.w) {
                    sectors[i] = max(sectors[i], dot(normalize(n_xyz_d.xyz-xyz_d.xyz), v));
                }
            }
        }

        w /= 2;
        h /= 2;
        c /= 2;
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
    float z = -texelFetch(pyramid, center, 0).z;
    float occ = lod_occlusion(center);
    map.r = occ > occlusion_threshold ? 0.0 : 1.0;
    map.g = occ > occlusion_threshold ? 1.0 : ((z - nf.x) / (nf.y - nf.x));
}
