#version 460
#define KERNEL_RADIUS 1

layout(location=0) uniform sampler2D visibility_map;
layout(location=1) uniform bool skip;
layout(location=2) uniform int width;
layout(location=3) uniform int height;
layout(location=4) uniform float sigma_depth;

layout(location=0) in vec2 uv;

layout(location=0) out float out_depth;

const float sigma_pixel = 4.5;
const float sigma_px_inv = 0.5 / (sigma_pixel*sigma_pixel);

float
bilateral_filter(ivec2 center) {
    float d0 = texelFetch(visibility_map, center, 0).g;
    float weighted = 0.0;
    float weight_sum = 0.0;
    float sigma_d_inv = 0.5 / (sigma_depth*sigma_depth);

    if (d0 < 0.9999) {
        for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x) {
            int i = center.x + x;
            if (i < 0 || i >= width) {
                continue;
            }
            for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y) {
                int j = center.y + y;
                if (j < 0 || j >= height) {
                    continue;
                }

                float depth = texelFetch(visibility_map, ivec2(i, j), 0).g;
                if (depth < 0.9999) {
                    float dpx = x*x + y*y;
                    float dd = depth - d0;
                    dd *= dd;
                    dd = 0.0;
                    float weight = exp(-dpx * sigma_px_inv - dd * sigma_d_inv);
                    weight_sum += weight;
                    weighted += weight * depth;
                }
            }
        }
    }

    return (weight_sum > 0.0) ? (weighted / weight_sum) : d0;
}

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
    if (skip) {
        out_depth = texelFetch(visibility_map, center, 0).g;
        return;
    }
    out_depth = bilateral_filter(center);
}
