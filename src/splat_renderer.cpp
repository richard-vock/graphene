#include <splat_renderer.hpp>

#include <baldr/fullscreen_pass.hpp>
#include <baldr/render_pass.hpp>
#include <baldr/shader_program.hpp>
#include <baldr/texture.hpp>
#include <baldr/data_buffer.hpp>
#include <baldr/vertex_array.hpp>
using namespace baldr;

#include <renderable.hpp>

namespace graphene::detail {

struct splat_renderer::impl {
    shared<float> splat_radius;
    shared<vec4f_t> clear_color_inner;
    shared<vec4f_t> clear_color_outer;
    std::shared_ptr<shader_program> transform_vs;
    std::shared_ptr<shader_program> visibility_fs;
    std::shared_ptr<shader_program> accumulate_fs;
    std::shared_ptr<shader_program> normalize_fs;
    std::shared_ptr<render_pass> visibility_pass;
    std::shared_ptr<render_pass> accumulate_pass;
    std::shared_ptr<fullscreen_pass> normalize_pass;
    std::shared_ptr<texture> gbuffer;

    impl(shared<float> splat_radius,
         shared<vec4f_t> clear_color_inner,
         shared<vec4f_t> clear_color_outer)
        : splat_radius(splat_radius),
          clear_color_inner(clear_color_inner),
          clear_color_outer(clear_color_outer)
    {
        transform_vs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "splat_transform.vert",
                                                   GL_VERTEX_SHADER);
        visibility_fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "splat_visibility.frag",
                                                    GL_FRAGMENT_SHADER);
        visibility_pass = std::make_shared<render_pass>(transform_vs, visibility_fs);

        accumulate_fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "splat_accumulate.frag",
                                                    GL_FRAGMENT_SHADER);
        accumulate_pass = std::make_shared<render_pass>(transform_vs, accumulate_fs);

        normalize_fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "splat_normalize.frag",
                                                   GL_FRAGMENT_SHADER);
        normalize_pass = std::make_shared<baldr::fullscreen_pass>(normalize_fs);
    }

    void
    render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
           std::shared_ptr<texture> linear_output,
           std::shared_ptr<texture> depth_buffer,
           mat4f_t const& view_mat,
           mat4f_t const& proj_mat,
           vec4i_t const& viewport)
    {
        if (!gbuffer || gbuffer->width() != viewport[2] || gbuffer->height() != viewport[3]) {
            gbuffer = texture::rgba32f(viewport[2], viewport[3]);
        }
        gbuffer->clear(vec4f_t(0.f, 0.f, 0.f, 0.f));

        transform_vs->uniform("view_mat") = view_mat;
        transform_vs->uniform("proj_mat") = proj_mat;
        transform_vs->uniform("viewport") = viewport;
        visibility_fs->uniform("viewport") = viewport;
        visibility_fs->uniform("proj_mat") = proj_mat;
        accumulate_fs->uniform("viewport") = viewport;
        accumulate_fs->uniform("proj_mat") = proj_mat;

        mat3f_t normal_mat = view_mat.topLeftCorner<3, 3>();

        glEnable(GL_PROGRAM_POINT_SIZE);
        visibility_pass->render(
            render_options{.depth_attachment = depth_buffer,
                           .depth_test = true,
                           .depth_write = true,
                           .clear_depth = 1.f,
                           .clear_color = vec4f_t(0.f, 0.f, 0.f, 1.f)},
            [&](auto vs, auto fs) {
                for (auto&& [name, obj] : objects) {
                    if (obj->hidden() || obj->render_mode() != render_mode_t::splats) {
                        continue;
                    }

                    vs->uniform("model_mat") = obj->transform();
                    mat3f_t nmat = normal_mat * obj->transform().template topLeftCorner<3, 3>();
                    vs->uniform("normal_mat") = nmat;
                    float r = splat_radius.get() * obj->resolution();
                    vs->uniform("splat_radius") = r;
                    fs->uniform("splat_radius") = r;

                    if (obj->vertex_count()) {
                        obj->vertex_array()->bind();
                        glDrawElements(GL_POINTS, obj->vertex_count(), GL_UNSIGNED_INT, 0);
                    }
                }
            });

        accumulate_pass->render(
            render_options{.output = {{"color", gbuffer}},
                           .depth_attachment = depth_buffer,
                           .depth_test = true,
                           .depth_write = false},
            [&](auto vs, auto fs) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                for (auto&& [name, obj] : objects) {
                    if (obj->hidden() || obj->render_mode() != render_mode_t::splats) {
                        continue;
                    }

                    vs->uniform("model_mat") = obj->transform();
                    vs->uniform("normal_mat") =
                        normal_mat * obj->transform().template topLeftCorner<3, 3>();
                    float r = splat_radius.get() * obj->resolution();
                    vs->uniform("splat_radius") = r;
                    fs->uniform("splat_radius") = r;

                    fs->uniform("use_texture") = static_cast<bool>(obj->texture());
                    if (obj->texture()) {
                        fs->sampler("tex") = *obj->texture();
                    }

                    if (obj->vertex_count()) {
                        obj->vertex_array()->bind();
                        glDrawElements(GL_POINTS, obj->vertex_count(), GL_UNSIGNED_INT, 0);
                    }
                }
                glDisable(GL_BLEND);
            });

        normalize_pass->render(
            render_options{.input = {{"gbuffer", gbuffer}}, .output = {{"color", linear_output}}},
            "background_inner",
            *clear_color_inner,
            "background_outer",
            *clear_color_outer);
    }

    void
    bind_vertex_array(vertex_array& vao, data_buffer& vbo)
    {
        transform_vs->buffer_binding(vao, "pos", "nrm", "fltcol", "uv", padding(sizeof(float))) =
            vbo;
    }
};

splat_renderer::splat_renderer(shared<float> splat_radius,
                               shared<vec4f_t> clear_color_inner,
                               shared<vec4f_t> clear_color_outer)
{
    impl_ = std::make_unique<impl>(splat_radius, clear_color_inner, clear_color_outer);
}

splat_renderer::~splat_renderer() = default;

void
splat_renderer::render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
                       std::shared_ptr<texture> linear_output,
                       std::shared_ptr<texture> depth_buffer,
                       mat4f_t const& view_mat,
                       mat4f_t const& proj_mat,
                       vec4i_t const& viewport)
{
    impl_->render(objects, linear_output, depth_buffer, view_mat, proj_mat, viewport);
}

void
splat_renderer::bind_vertex_array(std::shared_ptr<vertex_array> vao,
                                  std::shared_ptr<data_buffer> vbo)
{
    impl_->bind_vertex_array(*vao, *vbo);
}

}  // namespace graphene::detail
