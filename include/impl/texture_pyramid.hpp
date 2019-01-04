#pragma once

#include <graphene/common.hpp>
#include <baldr/baldr.hpp>

namespace graphene::detail {

class texture_pyramid {
public:
    texture_pyramid(GLenum format, GLenum internal_format, std::shared_ptr<baldr::shader_program> filter);

    void
    build();

    void
    reshape(const vec4i_t& vp);

    std::shared_ptr<baldr::texture>
    texture();

    uint32_t
    max_level() const;

protected:
    GLenum format_;
    GLenum internal_format_;
    std::shared_ptr<baldr::shader_program> filter_;
    bool init_;
    std::unique_ptr<baldr::fullscreen_pass> pass_;
    std::shared_ptr<baldr::texture> tex_;
    vec4i_t vp_;
    uint32_t max_level_;
};

} // graphene::detail
