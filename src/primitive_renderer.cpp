#include <primitive_renderer.hpp>

// #include <baldr/fullscreen_pass.hpp>
#include <baldr/render_pass.hpp>
#include <baldr/shader_program.hpp>
#include <baldr/texture.hpp>
// #include <baldr/data_buffer.hpp>
// #include <baldr/vertex_array.hpp>
using namespace baldr;

#include <renderable.hpp>

namespace graphene::detail {

struct primitive_renderer::impl {
    shared<float> splat_radius;
    shared<vec4f_t> clear_color;
    std::shared_ptr<shader_program> transform_vs;
    std::shared_ptr<shader_program> geometry_fs;
    std::shared_ptr<render_pass> pass;

    impl()
    {
        transform_vs = shader_program::load_binary(
            GRAPHENE_SHADER_ROOT + "solid_transform.vert",
            GL_VERTEX_SHADER);
        geometry_fs = shader_program::load_binary(
            GRAPHENE_SHADER_ROOT + "solid_geometry.frag",
            GL_FRAGMENT_SHADER);
        pass = std::make_shared<render_pass>(transform_vs, geometry_fs);
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

        pass->render(render_options{
                .output = { { "color", linear_output } },
                .depth_attachment = depth_buffer,
                .depth_test = true,
                .depth_write = true
            },
            [&] (auto vs, auto fs) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                for (auto && [name, obj] : objects) {
                    if (obj->hidden() || obj->render_mode() == render_mode_t::splats) {
                        continue;
                    }

                    vs->uniform("model_mat") = obj->transform();
                    vs->uniform("normal_mat") = normal_mat * obj->transform().template topLeftCorner<3,3>();

                    fs->uniform("use_texture") = static_cast<bool>(obj->texture());
                    if (obj->texture()) {
                        fs->sampler("tex") = *obj->texture();
                    }

                    GLuint mode = GL_TRIANGLES;
                    if (obj->render_mode() == render_mode_t::lines) {
                        mode = GL_LINES;
                    } else if (obj->render_mode() == render_mode_t::line_strip) {
                        mode = GL_LINE_STRIP;
                    }

                    if (obj->vertex_count()) {
                        obj->vertex_array()->bind();
                        glDrawElements(mode, obj->vertex_count(), GL_UNSIGNED_INT, 0);
                    }
                }
                glDisable(GL_BLEND);
            }
        );

    }

    // void
    // bind_vertex_array(vertex_array& vao, data_buffer& vbo)
    // {
    //     transform_vs->buffer_binding(vao, "pos", "nrm", "fltcol", "uv", padding(sizeof(float))) =
    //         vbo;
    // }
};

primitive_renderer::primitive_renderer()
{
    impl_ = std::make_unique<impl>();
}

primitive_renderer::~primitive_renderer() = default;

void
primitive_renderer::render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
                           std::shared_ptr<texture> linear_output,
                           std::shared_ptr<texture> depth_buffer,
                           mat4f_t const& view_mat,
                           mat4f_t const& proj_mat)
{
    impl_->render(objects, linear_output, depth_buffer, view_mat, proj_mat);
}

// void
// primitive_renderer::bind_vertex_array(std::shared_ptr<vertex_array> vao,
//                                   std::shared_ptr<data_buffer> vbo)
// {
//     impl_->bind_vertex_array(*vao, *vbo);
// }

}  // namespace graphene::detail
