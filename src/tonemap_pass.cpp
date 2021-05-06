#include <tonemap_pass.hpp>

#include <baldr/texture.hpp>
#include <baldr/shader_program.hpp>
#include <baldr/fullscreen_pass.hpp>
#include <kernel.hpp>

using namespace baldr;


namespace graphene::detail {

struct tonemap_pass::impl {
    shared<bool> enabled;
    shared<bool> needs_rebuild;
    shared<float> film_slope;
    shared<float> film_shoulder;
    shared<float> film_toe;
    shared<float> film_black_clip;
    shared<float> film_white_clip;
    kernel precomputation;
    std::shared_ptr<texture> lut;

    std::shared_ptr<baldr::shader_program> fs_;
    std::shared_ptr<baldr::fullscreen_pass> pass_;
    std::shared_ptr<baldr::shader_program> to_tex_fs_;
    std::shared_ptr<baldr::fullscreen_pass> to_tex_pass_;

    impl(shared<bool> enabled,
         shared<bool> needs_rebuild,
         shared<float> film_slope,
         shared<float> film_shoulder,
         shared<float> film_toe,
         shared<float> film_black_clip,
         shared<float> film_white_clip) :
         enabled(enabled),
         needs_rebuild(needs_rebuild),
         film_slope(film_slope),
         film_shoulder(film_shoulder),
         film_toe(film_toe),
         film_black_clip(film_black_clip),
         film_white_clip(film_white_clip),
         precomputation("precompute_tonemap_lut")
    {
        fs_ = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "tonemap.frag", GL_FRAGMENT_SHADER);
        pass_ = std::make_shared<baldr::fullscreen_pass>(fs_);
    }

    void
    rebuild()
    {
        if (!needs_rebuild.get()) return;

        lut = texture::rgba32f(32,32,32);
        lut->set_filter({GL_LINEAR, GL_LINEAR});
        precomputation.image("lut") = *lut;
        precomputation.uniform("film_slope") = film_slope.get();
        precomputation.uniform("film_shoulder") = film_shoulder.get();
        precomputation.uniform("film_toe") = film_toe.get();
        precomputation.uniform("film_black_clip") = film_black_clip.get();
        precomputation.uniform("film_white_clip") = film_white_clip.get();
        precomputation.launch3(32, 32, 32);
        precomputation.sync();
        needs_rebuild = false;
    }

    void
    render(std::shared_ptr<texture> linear_col,
           std::shared_ptr<texture> render_to_texture)
    {
        rebuild();

        if (render_to_texture && !to_tex_pass_) {
            to_tex_fs_ = shader_program::load_binary(GRAPHENE_SHADER_ROOT + "tonemap.frag",
                                                     GL_FRAGMENT_SHADER);
            to_tex_pass_ = std::make_shared<baldr::fullscreen_pass>(to_tex_fs_);
        }

        if (render_to_texture) {
            // auto depth_attachment = texture::depth32f(render_to_texture->width(), render_to_texture->height());
            to_tex_pass_->render(render_options{
                .input = {{"linear_col", linear_col},
                          {"tonemap_lut", lut}},
                .output = {{"color", render_to_texture}},
            }, "use_gamma_only", !enabled.get());
        } else {
            pass_->render(render_options{
                .input = {{"linear_col", linear_col},
                          {"tonemap_lut", lut}},
            }, "use_gamma_only", !enabled.get());
        }
    }
};

tonemap_pass::tonemap_pass(shared<bool> enabled,
                           shared<bool> needs_rebuild,
                           shared<float> film_slope,
                           shared<float> film_shoulder,
                           shared<float> film_toe,
                           shared<float> film_black_clip,
                           shared<float> film_white_clip)
{
    impl_ = std::make_unique<impl>(enabled,
                                   needs_rebuild,
                                   film_slope,
                                   film_shoulder,
                                   film_toe,
                                   film_black_clip,
                                   film_white_clip);
}

tonemap_pass::~tonemap_pass() = default;

void
tonemap_pass::render(std::shared_ptr<texture> linear_col,
                     std::shared_ptr<texture> render_to_texture)
{
    impl_->render(linear_col, render_to_texture);
}

} // graphene::detail
