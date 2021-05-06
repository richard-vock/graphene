#include <box.hpp>

namespace graphene {

box::box(const bbox3f_t& bounds, vec4f_t color)
{
    std::vector<vec3f_t> vertices(24);
    // front
    vertices[ 0] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.min()[2]);
    vertices[ 1] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.min()[2]);
    vertices[ 2] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.min()[2]);
    vertices[ 3] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.min()[2]);
    vertices[ 4] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.min()[2]);
    vertices[ 5] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.min()[2]);
    vertices[ 6] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.min()[2]);
    vertices[ 7] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.min()[2]);
    // back
    vertices[ 8] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.max()[2]);
    vertices[ 9] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.max()[2]);
    vertices[10] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.max()[2]);
    vertices[11] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.max()[2]);
    vertices[12] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.max()[2]);
    vertices[13] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.max()[2]);
    vertices[14] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.max()[2]);
    vertices[15] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.max()[2]);
    // sides
    vertices[16] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.min()[2]);
    vertices[17] = vec3f_t(bounds.min()[0], bounds.min()[1], bounds.max()[2]);
    vertices[18] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.min()[2]);
    vertices[19] = vec3f_t(bounds.max()[0], bounds.min()[1], bounds.max()[2]);
    vertices[20] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.min()[2]);
    vertices[21] = vec3f_t(bounds.max()[0], bounds.max()[1], bounds.max()[2]);
    vertices[22] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.min()[2]);
    vertices[23] = vec3f_t(bounds.min()[0], bounds.max()[1], bounds.max()[2]);

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
        vbo_data[i * 10 + 6] = pack_rgba(color);
        ibo_data[i] = i;
        bbox_.extend(vertices[i]);
    }
    vbo_ = std::make_shared<baldr::data_buffer>(vbo_data, GL_STATIC_DRAW);
    ibo_ = std::make_shared<baldr::data_buffer>(ibo_data, GL_STATIC_DRAW);
}

render_mode_t
box::render_mode() const
{
    return render_mode_t::lines;
}

bool
box::shaded() const
{
    return false;
}

std::shared_ptr<baldr::data_buffer>
box::vertex_buffer() const
{
    return vbo_;
}

std::shared_ptr<baldr::data_buffer>
box::index_buffer() const
{
    return ibo_;
}

}  // namespace graphene
