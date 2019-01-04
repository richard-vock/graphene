#version 460

#define M_PI 3.1415926535897932384626433832795

layout(location=0) uniform sampler2D gbuffer;

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

void main() {
    ivec2 center = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));

    vec3 nrm = texture(gbuffer, uv).rgb;

    if (length(nrm) < 0.5) discard;

    //float phi = (atan(nrm.y, nrm.x) + M_PI) / (2.0*M_PI);
    //float theta = asin(nrm.z) / M_PI + 0.5;
    color = vec4(0.5 * nrm + 0.5, 1.0);
}
