#include <cross.hpp>

namespace graphene {

cross::cross(const vec3f_t& pos, float size)
    : cross(pos, size, vec4f_t(1.f, 0.f, 0.f, 1.f), vec4f_t(0.f, 1.f, 0.f, 1.f), vec4f_t(0.f, 0.f, 1.f, 1.f))
{
}

cross::cross(const vec3f_t& pos, float size, const vec4f_t& color)
    : cross(pos, size, color, color, color)
{
}

cross::cross(const vec3f_t& pos,
             float size,
             const vec4f_t& col_x,
             const vec4f_t& col_y,
             const vec4f_t& col_z)
{
    std::vector<vec3f_t> vertices(6);
    // front
    vertices[0] = pos - size * vec3f_t::UnitX();;
    vertices[1] = pos + size * vec3f_t::UnitX();;
    vertices[2] = pos - size * vec3f_t::UnitY();;
    vertices[3] = pos + size * vec3f_t::UnitY();;
    vertices[4] = pos - size * vec3f_t::UnitZ();;
    vertices[5] = pos + size * vec3f_t::UnitZ();;
    std::vector<float> colors = {
        pack_rgba(col_x),
        pack_rgba(col_y),
        pack_rgba(col_z)
    };

    count_ = vertices.size();
    std::vector<float> vbo_data(count_ * 10, 0.f);
    std::vector<uint32_t> ibo_data(count_, 0.f);
    for (uint32_t i = 0; i < vertices.size(); ++i) {
        vbo_data[i * 10 + 0] = vertices[i][0];
        vbo_data[i * 10 + 1] = vertices[i][1];
        vbo_data[i * 10 + 2] = vertices[i][2];
        //vbo_data[i * 10 + 3] = 0.0f;
        //vbo_data[i * 10 + 4] = 0.0f;
        vbo_data[i * 10 + 5] = 1.0f;
        vbo_data[i * 10 + 6] = colors[i / 2];
        ibo_data[i] = i;
        bbox_.extend(vertices[i]);
    }
    vbo_ = std::make_shared<baldr::data_buffer>(vbo_data, GL_STATIC_DRAW);
    ibo_ = std::make_shared<baldr::data_buffer>(ibo_data, GL_STATIC_DRAW);
}

render_mode_t
cross::render_mode() const
{
    return render_mode_t::lines;
}

bool
cross::shaded() const
{
    return false;
}

std::shared_ptr<baldr::data_buffer>
cross::vertex_buffer() const
{
    return vbo_;
}

std::shared_ptr<baldr::data_buffer>
cross::index_buffer() const
{
    return ibo_;
}

}  // namespace graphene
