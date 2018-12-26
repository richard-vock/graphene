#pragma once

#include "common.hpp"

namespace graphene {

enum class render_mode_t : int
{
    splats,
    triangles,
    lines,
    line_strip
};

typedef union
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    float packed;
} packed_color_t;

float
pack_rgba(const vec4f_t& rgba);

vec4f_t
unpack_rgba(const float packed);

class renderable
{
public:
    // rows: X Y Z NX NY NZ COL U V CURV
    typedef Eigen::Matrix<float, Eigen::Dynamic, 10, Eigen::RowMajor> data_matrix_t;

public:
    renderable();

    virtual ~renderable() = default;

    virtual std::optional<std::vector<uint8_t>>
    texture() const = 0;

    virtual vec2i_t
    texture_size() const = 0;

    virtual render_mode_t
    render_mode() const = 0;

    virtual bool
    shaded() const = 0;

    virtual data_matrix_t
    data_matrix() const = 0;

    virtual std::vector<uint32_t>
    vertex_indices() const = 0;

    const mat4f_t&
    transform() const;

    mat4f_t&
    transform();

    void
    set_transform(const mat4f_t& t);

    void
    move(const mat4f_t& t);

protected:
    mat4f_t transform_;
};

}  // namespace graphene

#include "renderable.ipp"
