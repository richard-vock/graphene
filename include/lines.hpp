#pragma once

#include "renderable.hpp"

namespace graphene {

class lines : public renderable
{
public:
    lines(const std::vector<vec3f_t>& vertices, vec4f_t color, bool strip);

    lines(const std::vector<vec3f_t>& vertices, std::vector<vec4f_t> colors, bool strip);

    virtual ~lines() = default;

    virtual render_mode_t
    render_mode() const;

    virtual bool
    shaded() const;

    virtual std::shared_ptr<baldr::data_buffer>
    vertex_buffer() const;

    virtual std::shared_ptr<baldr::data_buffer>
    index_buffer() const;

protected:
    bool strip_;
    std::shared_ptr<baldr::data_buffer> vbo_;
    std::shared_ptr<baldr::data_buffer> ibo_;
    uint32_t count_;
    bbox3f_t bbox_;
};

} // graphene
