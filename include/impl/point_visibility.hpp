#pragma once

#include <baldr/baldr.hpp>
#include <graphene/common.hpp>
#include <graphene/shared.hpp>

namespace graphene::detail {

class point_visibility {
public:
    struct parameters {
        shared<float> occlusion_threshold = 1.f;
        shared<bool> fill = false;
    };

public:
    point_visibility(const parameters& params);

    virtual ~point_visibility();

    void render(const mat4f_t& projection_matrix, const vec4i_t& viewport, const vec2f_t& near_plane_size);

    void reshape(const vec4i_t& vp, std::shared_ptr<baldr::texture> input_depth);

    std::shared_ptr<baldr::texture>
    output();

protected:
    parameters params_;
    std::shared_ptr<baldr::texture> input_depth_;
    std::shared_ptr<baldr::texture> output_depth_;
    std::shared_ptr<baldr::texture> work_depth_;
    std::shared_ptr<baldr::shader_program> visibility_shader_;
    std::shared_ptr<baldr::shader_program> anisotropic_fill_shader_;
    std::shared_ptr<baldr::fullscreen_pass> visibility_pass_;
    std::shared_ptr<baldr::fullscreen_pass> anisotropic_fill_pass_;
};

} // graphene::detail
