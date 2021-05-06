#include <trivial_point_renderer.hpp>

#include <baldr/fullscreen_pass.hpp>
#include <baldr/render_pass.hpp>
#include <baldr/shader_program.hpp>
#include <baldr/texture.hpp>
#include <baldr/data_buffer.hpp>
#include <baldr/vertex_array.hpp>
using namespace baldr;

#include <renderable.hpp>

namespace graphene::detail {

struct trivial_point_renderer::impl {
    std::shared_ptr<shader_program> transform_vs;
    std::shared_ptr<shader_program> trivial_fs;
    std::shared_ptr<render_pass> pass;

    impl()
    {
        transform_vs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "point_transform.vert",
                                                   GL_VERTEX_SHADER);
        trivial_fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "point_render.frag",
                                                 GL_FRAGMENT_SHADER);
        pass = std::make_shared<render_pass>(transform_vs, trivial_fs);
    }

    void
    render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
           std::shared_ptr<texture> linear_output,
           std::shared_ptr<texture> depth_buffer,
           mat4f_t const& view_mat,
           mat4f_t const& proj_mat)
    {
        transform_vs->uniform("view_mat") = view_mat;
        transform_vs->uniform("proj_mat") = proj_mat;

        mat3f_t normal_mat = view_mat.topLeftCorner<3, 3>();

        pass->render(
            render_options{.output = {{"color", linear_output}},
                           .depth_attachment = depth_buffer,
                           .depth_test = true,
                           .depth_write = true,
                           .clear_depth = 1.f},
            [&](auto vs, auto) {
                for (auto&& [name, obj] : objects) {
                    if (obj->hidden() || obj->render_mode() != render_mode_t::splats) {
                        continue;
                    }

                    vs->uniform("model_mat") = obj->transform();
                    mat3f_t nmat = normal_mat * obj->transform().template topLeftCorner<3, 3>();
                    vs->uniform("normal_mat") = nmat;

                    if (obj->vertex_count()) {
                        obj->vertex_array()->bind();
                        glDrawElements(GL_POINTS, obj->vertex_count(), GL_UNSIGNED_INT, 0);
                    }
                }
            });
    }

    void
    bind_vertex_array(vertex_array& vao, data_buffer& vbo)
    {
        transform_vs->buffer_binding(vao, "pos", "nrm", "fltcol", "uv", padding(sizeof(float))) =
            vbo;
    }
};

trivial_point_renderer::trivial_point_renderer()
{
    impl_ = std::make_unique<impl>();
}

trivial_point_renderer::~trivial_point_renderer() = default;

void
trivial_point_renderer::render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
                               std::shared_ptr<texture> linear_output,
                               std::shared_ptr<texture> depth_buffer,
                               mat4f_t const& view_mat,
                               mat4f_t const& proj_mat)
{
    impl_->render(objects, linear_output, depth_buffer, view_mat, proj_mat);
}

void
trivial_point_renderer::bind_vertex_array(std::shared_ptr<vertex_array> vao,
                                  std::shared_ptr<data_buffer> vbo)
{
    impl_->bind_vertex_array(*vao, *vbo);
}

}  // namespace graphene::detail
