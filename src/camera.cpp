#include <graphene/camera.hpp>

namespace graphene {

void
camera::update(vec3f_t translational, vec3f_t rotational)
{
    std::lock_guard<std::mutex> lock(mut_);
    update_(translational, rotational);
    view_matrix_ = get_view_matrix_();
}

Eigen::ParametrizedLine<float, 3>
camera::pick_ray(const vec2i_t& px)
{
    mat4f_t inv = (projection_matrix_ * view_matrix_).inverse();

    vec4f_t tmp(static_cast<float>(px[0]), static_cast<float>(px[1]), near_,
                1.f);

    tmp[0] = (tmp[0] - static_cast<float>(viewport_[0])) /
             static_cast<float>(viewport_[2]);
    tmp[1] = (tmp[1] - static_cast<float>(viewport_[1])) /
             static_cast<float>(viewport_[3]);
    tmp = 2.f * tmp - vec4f_t::Ones();

    vec3f_t origin = position();
    vec4f_t dir = inv * tmp;
    dir = dir / dir[3] - origin.homogeneous();

    return Eigen::ParametrizedLine<float, 3>(origin, dir.head(3).normalized());
}

camera::camera(std::shared_ptr<event_manager> events, vec4i_t viewport,
               float fov, float near, float far,
               std::function<void(vec3f_t, vec3f_t)> update_callback,
               std::function<mat4f_t()> get_view_matrix_callback)
    : events_(events),
      fov_(fov),
      near_(near),
      far_(far),
      update_(std::move(update_callback)),
      get_view_matrix_(std::move(get_view_matrix_callback)),
      projection_matrix_changed_(false)
{
    // clang-format off
    math_to_opengl_ <<  1.f, 0.f, 0.f, 0.f,
                        0.f, 0.f, 1.f, 0.f,
                        0.f, -1.f, 0.f, 0.f,
                        0.f, 0.f, 0.f, 1.f;
    opengl_to_math_ <<  1.f, 0.f, 0.f, 0.f,
                        0.f, 0.f, -1.f, 0.f,
                        0.f, 1.f, 0.f, 0.f,
                        0.f, 0.f, 0.f, 1.f;
    // clang-format on
    view_matrix_ = get_view_matrix_();
    auto reshape = [&] (vec4i_t viewport) {
        vec2f_t size = viewport.tail(2).template cast<float>();
        float aspect = size[0] / size[1];

        float fovtan = std::tan(fov_ * M_PI / 360.f);

        // float f = 1.0 / fovtan;
        // frustum_width_ = 2.f*near_*aspect / f;
        // frustum_height_ = 2.f*near_ / f;

        float range = fovtan * near_;
        float left = -range * aspect;
        float right = range * aspect;
        float bottom = -range;
        float top = range;

        std::lock_guard<std::mutex> lock(mut_);
        viewport_ = std::move(viewport);
        projection_matrix_ = mat4f_t::Zero();
        projection_matrix_(0, 0) = (2.f * near_) / (right - left);
        projection_matrix_(1, 1) = (2.f * near_) / (top - bottom);
        projection_matrix_(2, 2) = -(far_ + near_) / (far_ - near_);
        projection_matrix_(3, 2) = -1.f;
        projection_matrix_(2, 3) = -(2.f * far_ * near_) / (far_ - near_);
        projection_matrix_changed_ = true;
    };
    reshape(std::move(viewport));

    events_->connect<events::mouse_scroll>([&] (int32_t delta, modifier) {
        update(vec3f_t(0.f, 0.f, delta), vec3f_t(0.f, 0.f, 0.f));
    });

    events->connect<events::mouse_drag>([&] (int btn, vec2f_t delta, vec2f_t, modifier mod) {
        if (btn == 1) {
            if (mod.ctrl) {
                update(vec3f_t(delta[0], delta[1], 0.f), vec3f_t(0.f, 0.f, 0.f));
            } else {
                update(vec3f_t(0.f, 0.f, 0.f), vec3f_t(delta[0], delta[1], 0.f));
            }
        }
    });

    events->connect<events::window_resize>(reshape);
}

}  // namespace graphene
