#version 460

layout(location=0) uniform sampler2D input_map;
layout(location=1) uniform int width;
layout(location=2) uniform int height;

layout(location=0) in vec2 uv;

layout(location=0) out vec2 output_map;

float
median_depth(ivec2 center);

float
update_depth(ivec2 center, float depth);

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
    vec2 inp = texelFetch(input_map, center, 0).rg;

    float visibility = inp.r;
    float depth = inp.g;
    if (visibility < 0.5) {
        // if depth < 1.0 we have valid pixel and update depth
        // otherwise initialize with median neighbor depth
        depth =
            inp.g < 1.0 ? update_depth(center, depth) : median_depth(center);
    }

    output_map.r = visibility;
    output_map.g = depth;
}

float
median_depth(ivec2 center) {
    int n_count = 0;
    float depths[8];

    float depth;
    for (int x = -1; x <= 1; ++x) {
        int i = center.x + x;
        if (i < 0 || i >= width) {
            continue;
        }
        for (int y = -1; y <= 1; ++y) {
            int j = center.y + y;
            if (j < 0 || j >= height || (x == 0 && y == 0)) {
                continue;
            }

            depth = texelFetch(input_map, ivec2(i, j), 0).g;
            if (depth < 1.0) {
                depths[n_count++] = depth;
            }
        }
    }

    if (n_count == 0) {
        // no valid neighbor, return invalid depth
        return 1.0;
    }

    // insertion sort
    for (int j = 0; j < n_count; j++) {
        // find min
        int min_depth = j;
        for (int i = j+1; i < n_count; i++) {
            if (depths[i] < depths[min_depth]) {
                min_depth = i;
            }
        }

        if (min_depth != j) {
            // swap
            float tmp = depths[j];
            depths[j] = depths[min_depth];
            depths[min_depth] = tmp;
        }
    }

    return depths[n_count / 2];
}

float
update_depth(ivec2 center, float depth) {
    float weighted_depth = 0.0;
    float weight_sum = 0.0;
    for (int x = -1; x <= 1; ++x) {
        int i = center.x + x;
        if (i < 0 || i >= width) {
            continue;
        }
        for (int y = -1; y <= 1; ++y) {
            int j = center.y + y;
            if (j < 0 || j >= height) {
                continue;
            }

            //vec2 n_inp = texelFetch(input_map, ivec2(i, j), 0).rg;
            //bool n_visible = n_inp.r > 0.5;
            float n_depth = texelFetch(input_map, ivec2(i, j), 0).g;

            if (n_depth < 1.0) {
                float px_factor = 1.0 - 0.5 * float(abs(x) + abs(y));
                float depth_factor = 1.0 - min(1.0, abs(depth - n_depth));
                float weight = px_factor * depth_factor;
                weighted_depth += weight * n_depth;
                weight_sum += weight;
            }
        }
    }

    return weighted_depth / weight_sum;
}
