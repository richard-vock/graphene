#include <impl/texture_pyramid.hpp>

using namespace baldr;

namespace graphene::detail {

texture_pyramid::texture_pyramid(std::shared_ptr<baldr::shader_program> filter) : filter_(filter), init_(false) {
    pass_ = std::make_unique<baldr::fullscreen_pass>(filter_);
}

void
texture_pyramid::build(std::shared_ptr<baldr::texture> depth_map, const mat4f_t& proj_mat, const vec2f_t& near_plane_size) {
    terminate_unless(init_, "texture_pyramid::reshape() needs to be called at least once before build()");
    for (int level = 0; level <= static_cast<int>(max_level_); ++level) {
        if (level) {
            int width = std::max(1, vp_[2] / static_cast<int>(std::pow(2.f, static_cast<float>(level))));
            int height = std::max(1, vp_[3] / static_cast<int>(std::pow(2.f, static_cast<float>(level))));
            glViewport(vp_[0], vp_[1], width, height);
        }
        if (level) {
            tex_->set_max_level(level-1);
        }
        pass_->render(
            render_options{
                .input = {{"pyramid", level ? tex_ : depth_map}},
                .output = {{"out_pyramid", texture_image{tex_, level}}},
                .depth_write = false
            },
            "width", real_vp_[2],
            "height", real_vp_[3],
            "level", level,
            "proj_mat", proj_mat,
            "near_size", near_plane_size
        );
        glTextureBarrier();
    }
    tex_->set_max_level(max_level_);
    glViewport(real_vp_[0], real_vp_[1], real_vp_[2], real_vp_[3]);
}

void
texture_pyramid::reshape(const vec4i_t& vp) {
    vp_[0] = vp[0];
    vp_[1] = vp[1];
    vp_[2] = vp_[3] = 1;
    while (vp_[2] < vp[2]) {
        vp_[2] *= 2;
    }
    while (vp_[3] < vp[3]) {
        vp_[3] *= 2;
    }
    max_level_ = std::log2(vp_.tail(2).minCoeff());
    pdebug("max: {}, vp: {}", max_level_, vp_.transpose());
    tex_ = std::make_shared<baldr::texture>(vp_[2], vp_[3], texture_specification{
        .format = GL_RGBA,
        .internal_format = GL_RGBA32F,
        .filter = {GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
        .levels = max_level_+1u
    });
    real_vp_ = vp;
    init_ = true;
}

std::shared_ptr<baldr::texture>
texture_pyramid::texture() {
    return tex_;
}

uint32_t
texture_pyramid::max_level() const {
    return max_level_;
}

} // graphene::detail
