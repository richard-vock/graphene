#pragma once

#include "renderable.hpp"

namespace graphene {

class lines : public renderable
{
public:
    lines(const std::vector<vec3f_t>& vertices, vec4f_t color, bool strip);

    lines(const std::vector<vec3f_t>& vertices, std::vector<vec4f_t> colors, bool strip);

    virtual ~lines() = default;

    virtual std::optional<std::vector<uint8_t>>
    texture() const;

    virtual vec2i_t
    texture_size() const;

    virtual render_mode_t
    render_mode() const;

    virtual bool
    shaded() const;

    virtual data_matrix_t
    data_matrix() const;

    virtual std::vector<uint32_t>
    vertex_indices() const;

protected:
    bool strip_;
    data_matrix_t mat_;
    std::vector<uint32_t> indices_;
};

} // graphene
