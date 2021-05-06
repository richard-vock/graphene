#include <lines.hpp>

namespace graphene {

lines::lines(const std::vector<vec3f_t>& vertices, vec4f_t color, bool strip)
    : lines(vertices, std::vector<vec4f_t>(vertices.size(), color), strip)
{}

lines::lines(const std::vector<vec3f_t>& vertices, std::vector<vec4f_t> colors,
             bool strip)
    : strip_(strip)
{
    terminate_unless(vertices.size() == colors.size(),
                     "Vertex and color count do not match");
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
        vbo_data[i * 10 + 6] = pack_rgba(colors[i]);
        ibo_data[i] = i;
        bbox_.extend(vertices[i]);
    }
    vbo_ = std::make_shared<baldr::data_buffer>(vbo_data, GL_STATIC_DRAW);
    ibo_ = std::make_shared<baldr::data_buffer>(ibo_data, GL_STATIC_DRAW);
}

render_mode_t
lines::render_mode() const
{
    return strip_ ? render_mode_t::line_strip : render_mode_t::lines;
}

bool
lines::shaded() const
{
    return false;
}

std::shared_ptr<baldr::data_buffer>
lines::vertex_buffer() const
{
    return vbo_;
}

std::shared_ptr<baldr::data_buffer>
lines::index_buffer() const
{
    return ibo_;
}

}  // namespace graphene
