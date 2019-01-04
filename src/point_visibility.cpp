#include <impl/point_visibility.hpp>

using namespace baldr;

namespace graphene::detail {

constexpr uint32_t kernel_size = 13;

point_visibility::point_visibility(const parameters& params) : params_(params) {
    visibility_shader_ = shader_program::load(
        SHADER_ROOT + "visibility.frag", GL_FRAGMENT_SHADER);
    visibility_pass_ =
        std::make_shared<baldr::fullscreen_pass>(visibility_shader_);
    anisotropic_fill_shader_ = shader_program::load(
        SHADER_ROOT + "anisotropic_fill.frag", GL_FRAGMENT_SHADER);
    anisotropic_fill_pass_ =
        std::make_shared<baldr::fullscreen_pass>(anisotropic_fill_shader_);
}

point_visibility::~point_visibility() {
}

void
point_visibility::render(const mat4f_t& projection_matrix, const vec4i_t& viewport, const vec2f_t& near_plane_size, const vec2f_t& nf) {
    visibility_shader_->uniform("proj_mat") = projection_matrix;
    visibility_shader_->uniform("near_size") = near_plane_size;
    visibility_shader_->uniform("width") = viewport[2];
    visibility_shader_->uniform("height") = viewport[3];
    visibility_shader_->uniform("occlusion_threshold") = params_.occlusion_threshold;
    visibility_shader_->uniform("kernel_size") = kernel_size;
    visibility_shader_->uniform("nf") = nf;
    anisotropic_fill_shader_->uniform("width") = viewport[2];
    anisotropic_fill_shader_->uniform("height") = viewport[3];

    visibility_pass_->render(render_options{
        .input = {{"depth_tex", input_depth_}},
        .output = {{"map", output_depth_}},
        .depth_write = false,
        .clear_color = vec4f_t(0.f, 0.f, 0.f, 0.f)
    });

    if (params_.fill) {
        for (uint32_t i = 0; i < 2*kernel_size; ++i) {
            // we necessarily have an even iteration count
            // the final result will therefore be in the original
            // input map (visibility_map_)
            anisotropic_fill_pass_->render(render_options{
                .input = {{"input_map", (i % 2 == 0) ? output_depth_ : work_depth_ }},
                .output = {{"output_map", (i % 2 == 0) ? work_depth_ : output_depth_}},
                .depth_write = false,
                .clear_color = vec4f_t(0.f, 0.f, 0.f, 0.f)
            });
        }
    }
}

void
point_visibility::reshape(const vec4i_t& vp, std::shared_ptr<texture> input_depth) {
    input_depth_ = input_depth;
    output_depth_ = texture::rg32f(vp[2], vp[3]);
    work_depth_ = texture::rg32f(vp[2], vp[3]);
}

std::shared_ptr<texture>
point_visibility::output() {
    return output_depth_;
}

} // graphene::detail
