#version 460

#include <util.comp>
#include <tonemap.comp>

layout(location=0) in vec3 c_eye;
layout(location=1) in vec3 c_eye_nrm;
layout(location=2) in vec4 rgba;
layout(location=3) in vec2 tex_uv;

layout(location=0) out vec4 color;

void main() {
    color = vec4(inv_gamma_map(rgba.rgb), rgba.a);
}
