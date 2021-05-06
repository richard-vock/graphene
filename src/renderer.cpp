#include <renderer.hpp>

#include <numeric>

#include <baldr/texture.hpp>
using namespace baldr;

#include <camera.hpp>
#include <depth_vis_pass.hpp>
#include <primitive_renderer.hpp>
#include <splat_renderer.hpp>
#include <tonemap_pass.hpp>

namespace graphene::detail {

vec2f_t
correct_near_far(const bbox3f_t& bbox, const ray_t& view, const mat4f_t& transform)
{
    float min_t = std::numeric_limits<float>::max();
    float max_t = 0.f;
    auto add = [&](bbox3f_t::CornerType c) {
        vec3f_t pos = (transform * bbox.corner(c).homogeneous()).head(3);
        float t = view.direction().dot(pos - view.origin());
        max_t = std::max(max_t, t);
        min_t = std::min(min_t, t);
    };

    add(bbox3f_t::BottomLeftFloor);
    add(bbox3f_t::BottomRightFloor);
    add(bbox3f_t::TopLeftFloor);
    add(bbox3f_t::TopRightFloor);
    add(bbox3f_t::BottomLeftCeil);
    add(bbox3f_t::BottomRightCeil);
    add(bbox3f_t::TopLeftCeil);
    add(bbox3f_t::TopRightCeil);

    return vec2f_t(min_t, max_t);
}

struct renderer::impl {
    std::shared_ptr<event_manager> events_;
    std::shared_ptr<camera> cam_;

    std::shared_ptr<baldr::texture> depth_buffer_;
    std::shared_ptr<baldr::texture> linear_col_;

    std::shared_ptr<splat_renderer> splat_renderer_;
    std::shared_ptr<primitive_renderer> primitive_renderer_;
    std::shared_ptr<tonemap_pass> tonemap_pass_;
    std::shared_ptr<depth_vis_pass> depth_vis_pass_;

    std::mutex data_mutex_;
    std::map<std::string, std::shared_ptr<renderable>> objects_;

    parameters params_;

    impl(std::shared_ptr<event_manager> events,
         std::shared_ptr<camera> cam,
         const parameters& params)
        : events_(events), cam_(cam), params_(params)
    {
        splat_renderer_ = std::make_shared<splat_renderer>(
            params_.splat_radius, params.background_inner, params.background_outer);
        primitive_renderer_ = std::make_shared<primitive_renderer>();
        depth_vis_pass_ = std::make_shared<depth_vis_pass>();

        tonemap_pass_ = std::make_shared<tonemap_pass>(params_.tonemapping_enabled,
                                                       params_.rebuild_lut,
                                                       params_.film_slope,
                                                       params_.film_shoulder,
                                                       params_.film_toe,
                                                       params_.film_black_clip,
                                                       params_.film_white_clip);

        // add object
        events_->connect<events::add_object>(
            [&](std::string name, std::shared_ptr<renderable> obj) {
                std::lock_guard<std::mutex> lock(data_mutex_);

                objects_[name] = obj;
                if (obj->index_count()) {
                    splat_renderer_->bind_vertex_array(obj->vertex_array(), obj->vertex_buffer());
                }
            });

        // remove object
        events_->connect<events::remove_object>([&](std::string name) {
            std::lock_guard<std::mutex> lock(data_mutex_);
            if (auto it = objects_.find(name); it != objects_.end()) {
                objects_.erase(it);
            }
        });

        // hide object
        events_->connect<events::hide_object>([&](std::string name) {
            std::lock_guard<std::mutex> lock(data_mutex_);
            if (auto it = objects_.find(name); it != objects_.end()) {
                it->second->hide();
            }
        });

        // show object
        events_->connect<events::show_object>([&](std::string name) {
            std::lock_guard<std::mutex> lock(data_mutex_);
            if (auto it = objects_.find(name); it != objects_.end()) {
                it->second->show();
            }
        });

        // viewport dependent initialization
        auto reshape = [&](vec4i_t viewport) {
            // textures
            depth_buffer_ = texture::depth32f(viewport[2], viewport[3]);
            linear_col_ = texture::rgba32f(viewport[2], viewport[3]);
        };
        reshape(cam_->viewport());
        events_->connect<events::window_resize>(reshape);

        // vertex picking
        events_->connect<events::mouse_click>([&](int32_t btn, vec2f_t px, modifier) {
            if (btn != 0) return;  // left click only
            for (auto&& [name, obj] : objects_) {
                if (obj->pickable()) {
                    // fmt::print("picking in {}  --  btn: {}, px: {}, radius: {}\n", name, btn,
                    // px.transpose(), params_.pick_radius.get()); mirror pixel in y coord
                    vec2f_t nf(0.1f, 400.f);
                    vec4i_t viewport = cam_->viewport();
                    px[1] = static_cast<float>(viewport[3] - 1) - px[1];
                    if (auto idx = obj->pick_vertices(px,
                                                      cam_->view_matrix(),
                                                      cam_->projection_matrix(nf),
                                                      viewport,
                                                      params_.pick_radius.get())) {
                        events_->emit<events::picked_vertex>(name, *idx);
                    }
                }
            }
        });
    }

    void
    render(mat4f_t const& view_mat,
           mat4f_t const& proj_mat,
           std::shared_ptr<texture> render_to_texture,
           std::shared_ptr<texture> render_depth_to_texture)
    {
        vec4i_t viewport = cam_->viewport();

        // textures only used for render to texture
        std::shared_ptr<texture> depth_buffer = depth_buffer_;
        std::shared_ptr<texture> linear_col = linear_col_;
        if (render_to_texture || render_depth_to_texture) {
            int w =
                render_to_texture ? render_to_texture->width() : render_depth_to_texture->width();
            int h =
                render_to_texture ? render_to_texture->height() : render_depth_to_texture->height();
            viewport[0] = 0;
            viewport[1] = 0;
            viewport[2] = w;
            viewport[3] = h;

            depth_buffer =
                render_depth_to_texture ? render_depth_to_texture : texture::depth32f(w, h);
            linear_col = texture::rgba32f(w, h);
        }

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        linear_col->clear(params_.background_inner.get());

        std::lock_guard<std::mutex> lock(data_mutex_);

        splat_renderer_->render(objects_, linear_col, depth_buffer, view_mat, proj_mat, viewport);

        primitive_renderer_->render(objects_, linear_col, depth_buffer, view_mat, proj_mat);

        if (params_.render_depth_buffer.get()) {
            depth_vis_pass_->render(depth_buffer, proj_mat);
            return;
        }

        tonemap_pass_->render(linear_col, render_to_texture);
    }
};

renderer::renderer(std::shared_ptr<event_manager> events,
                   std::shared_ptr<camera> cam,
                   const parameters& params)
{
    impl_ = std::make_unique<impl>(events, cam, params);
}

renderer::~renderer() = default;

void
renderer::render(std::shared_ptr<texture> render_to_texture,
                 std::shared_ptr<texture> render_depth_to_texture)
{
    mat4f_t view_mat = impl_->cam_->view_matrix();
    mat4f_t proj_mat = impl_->cam_->projection_matrix(vec2f_t(0.1f, 400.f));
    impl_->render(view_mat, proj_mat, render_to_texture, render_depth_to_texture);
}

void
renderer::render(mat4f_t const& view_mat,
                 mat4f_t const& proj_mat,
                 std::shared_ptr<texture> render_to_texture,
                 std::shared_ptr<texture> render_depth_to_texture)
{
    impl_->render(view_mat, proj_mat, render_to_texture, render_depth_to_texture);
}

}  // namespace graphene::detail
