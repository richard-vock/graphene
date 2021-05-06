#pragma once

#include "renderable.hpp"

namespace graphene {

class cross : public renderable
{
public:
    cross(const vec3f_t& pos, float size);

    cross(const vec3f_t& pos, float size, const vec4f_t& color);

    cross(const vec3f_t& pos,
          float size,
          const vec4f_t& col_x,
          const vec4f_t& col_y,
          const vec4f_t& col_z);

    virtual ~cross() = default;

    virtual render_mode_t
    render_mode() const;

    virtual bool
    shaded() const;

    virtual std::shared_ptr<baldr::data_buffer>
    vertex_buffer() const;

    virtual std::shared_ptr<baldr::data_buffer>
    index_buffer() const;

protected:
    std::shared_ptr<baldr::data_buffer> vbo_;
    std::shared_ptr<baldr::data_buffer> ibo_;
    uint32_t count_;
    bbox3f_t bbox_;
};

} // graphene
