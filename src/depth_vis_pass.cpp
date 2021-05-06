#include <depth_vis_pass.hpp>

#include <baldr/texture.hpp>
#include <baldr/shader_program.hpp>
#include <baldr/fullscreen_pass.hpp>
using namespace baldr;

#include "common.hpp"

namespace graphene::detail {

struct depth_vis_pass::impl {
    std::shared_ptr<shader_program> fs;
    std::shared_ptr<fullscreen_pass> pass;
    std::shared_ptr<shader_program> to_tex_fs;
    std::shared_ptr<fullscreen_pass> to_tex_pass;

    impl()
    {
        fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "render_depth.frag", GL_FRAGMENT_SHADER);
        pass = std::make_shared<baldr::fullscreen_pass>(fs);
    }

    void
    render(std::shared_ptr<texture> depth_buffer,
           mat4f_t const& pmat,
           std::shared_ptr<texture> render_to_texture)
    {
        if (render_to_texture && !to_tex_pass) {
        }

        if (render_to_texture) {
            if (!to_tex_pass) {
                to_tex_fs = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "render_depth.frag", GL_FRAGMENT_SHADER);
                to_tex_pass = std::make_shared<baldr::fullscreen_pass>(to_tex_fs);
            }
            to_tex_pass->render(render_options{
                .input = {{"tex", depth_buffer}},
                .output = {{"color", render_to_texture}}
            }, "proj_mat", pmat);
        } else {
            pass->render(render_options{
                .input = {{"tex", depth_buffer}}
            }, "proj_mat", pmat);
        }
    }
};

depth_vis_pass::depth_vis_pass()
{
    impl_ = std::make_unique<impl>();
}

depth_vis_pass::~depth_vis_pass() = default;

void
depth_vis_pass::render(std::shared_ptr<texture> depth_buffer,
                       mat4f_t const& pmat,
                       std::shared_ptr<texture> render_to_texture)
{
    impl_->render(depth_buffer, pmat, render_to_texture);
}

} // graphene::detail
