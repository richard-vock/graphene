#include <graphene/lines.hpp>

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
    mat_ = data_matrix_t(vertices.size(), 10);
    indices_.resize(vertices.size());
    for (uint32_t i = 0; i < vertices.size(); ++i) {
        // pos
        mat_.block<1, 3>(i, 0) = vertices[i].transpose();
        // nrm
        mat_.block<1, 3>(i, 3) = vec3f_t::UnitZ();
        // col
        mat_(i, 6) = pack_rgba(colors[i]);
        // uv
        mat_.block<1, 2>(i, 7) = vec2f_t::Zero();
        // curv
        mat_(i, 9) = 0.f;
        indices_[i] = i;
    }
}

std::optional<std::vector<uint8_t>>
lines::texture() const
{
    return std::nullopt;
}

vec2i_t
lines::texture_size() const
{
    return vec2i_t::Zero();
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

renderable::data_matrix_t
lines::data_matrix() const
{
    return mat_;
}

std::vector<uint32_t>
lines::vertex_indices() const
{
    return indices_;
}

}  // namespace graphene
