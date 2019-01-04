#version 460

layout(location=0) uniform sampler2D pyramid;
layout(location=1) uniform int width;
layout(location=2) uniform int height;
layout(location=3) uniform int level;

layout(location=0) in vec2 uv;

layout(location=0) out float out_pyramid;

void main() {
    ivec2 px = ivec2(uv * vec2(2*width, 2*height));
    float ll = texelFetch(pyramid, px + ivec2(0,0), level-1).r;
    float lr = texelFetch(pyramid, px + ivec2(1,0), level-1).r;
    float ul = texelFetch(pyramid, px + ivec2(0,1), level-1).r;
    float ur = texelFetch(pyramid, px + ivec2(1,1), level-1).r;
    out_pyramid = min(min(ll,lr), min(ul, ur));
}
