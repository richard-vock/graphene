#pragma once

#include "common.hpp"

#include <baldr/data_buffer.hpp>
#include <baldr/vertex_array.hpp>
#include <baldr/texture.hpp>

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
// public:
    // rows: X Y Z NX NY NZ COL U V CURV
    // typedef Eigen::Matrix<float, Eigen::Dynamic, 10, Eigen::RowMajor> data_matrix_t;

public:
    renderable();

    virtual ~renderable() = default;

    virtual std::shared_ptr<baldr::texture>
    texture() const;

    virtual render_mode_t
    render_mode() const = 0;

    virtual bool
    shaded() const = 0;

    virtual std::shared_ptr<baldr::data_buffer>
    vertex_buffer() const = 0;

    virtual std::shared_ptr<baldr::data_buffer>
    index_buffer() const = 0;

    std::shared_ptr<baldr::vertex_array>
    vertex_array();

    uint32_t
    index_count() const;

    uint32_t
    vertex_count() const;

    bbox3f_t
    bounding_box();

    float
    resolution();

    const mat4f_t&
    transform() const;

    mat4f_t&
    transform();

    void
    set_transform(const mat4f_t& t);

    void
    move(const mat4f_t& t);

    bool
    pickable() const;

    void
    set_pickable(bool state);

    std::optional<uint32_t>
    pick_vertices(const vec2f_t& px,
                  const mat4f_t& view_matrix,
                  const mat4f_t& projection_matrix,
                  const vec4i_t& viewport,
                  float radius,
                  uint32_t max_candidates = 10);

    bool
    hidden() const;

    void
    hide();

    void
    show();

    void
    set_hidden(bool state);

protected:
    void
    compute_bbox_();

public:
    mat4f_t  transform_;
    bbox3f_t bbox_;
    std::shared_ptr<baldr::vertex_array> vao_;
    float res_;
    bool computed_bbox_;
    bool pickable_;
    bool hidden_;
};

}  // namespace graphene

#include "renderable.ipp"
