#include <impl/texture_pyramid.hpp>

using namespace baldr;

namespace graphene::detail {

texture_pyramid::texture_pyramid(GLenum format, GLenum internal_format, std::shared_ptr<baldr::shader_program> filter) : format_(format), internal_format_(internal_format), filter_(filter), init_(false) {
    pass_ = std::make_unique<baldr::fullscreen_pass>(filter_);
}

void
texture_pyramid::build() {
    terminate_unless(init_, "texture_pyramid::reshape() needs to be called at least once before build()");
    for (int level = 1; level <= static_cast<int>(max_level_); ++level) {
        int width = std::max(1, vp_[2] / static_cast<int>(std::pow(2.f, static_cast<float>(level))));
        int height = std::max(1, vp_[3] / static_cast<int>(std::pow(2.f, static_cast<float>(level))));
        glViewport(vp_[0], vp_[1], width, height);
        tex_->set_max_level(level-1);
        filter_->uniform("width") = width;
        filter_->uniform("height") = height;
        filter_->uniform("level") = level;
        pass_->render(render_options{
            .input = {{"pyramid", tex_}},
            .output = {{"out_pyramid", texture_image{tex_, level}}},
            .depth_write = false
        });
        glTextureBarrier();
    }
    glViewport(vp_[0], vp_[1], vp_[2], vp_[3]);
}

void
texture_pyramid::reshape(const vec4i_t& vp) {
    max_level_ = std::log2(vp.tail(2).minCoeff());
    tex_ = std::make_shared<baldr::texture>(vp[2], vp[3], texture_specification{
        .format = format_,
        .internal_format = internal_format_,
        .filter = {GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
        .levels = max_level_+1u
    });
    vp_ = vp;
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
