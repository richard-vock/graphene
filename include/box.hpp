#pragma once

#include "renderable.hpp"

namespace graphene {

class box : public renderable
{
public:
    box(const bbox3f_t& bounds, vec4f_t color);

    virtual ~box() = default;

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
