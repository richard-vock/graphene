#include <camera.hpp>
#include <fstream>

namespace fs = std::filesystem;

namespace graphene {

void
camera::update(vec3f_t translational, vec3f_t rotational)
{
    std::lock_guard<std::mutex> lock(mut_);
    update_(translational, rotational);
    view_matrix_ = get_view_matrix_();
}

ray_t
camera::pick_ray(const vec2i_t& px, const vec2f_t& near_far)
{
    mat4f_t inv = (this->projection_matrix(near_far) * view_matrix_).inverse();

    vec4f_t tmp(static_cast<float>(px[0]), static_cast<float>(px[1]), 0.001f,
                1.f);

    tmp[0] = (tmp[0] - static_cast<float>(viewport_[0])) /
             static_cast<float>(viewport_[2]);
    tmp[1] = (tmp[1] - static_cast<float>(viewport_[1])) /
             static_cast<float>(viewport_[3]);
    tmp = 2.f * tmp - vec4f_t::Ones();

    vec3f_t origin = position();
    vec4f_t dir = inv * tmp;
    dir = dir / dir[3] - origin.homogeneous();

    return ray_t(origin, dir.head(3).normalized());
}

camera::camera(std::shared_ptr<event_manager> events, vec4i_t viewport,
               shared<float> fov,
               std::function<void(vec3f_t, vec3f_t)> update_callback,
               std::function<mat4f_t()> get_view_matrix_callback,
               std::function<void(mat4f_t const&)> restore_model_callback)
    : events_(events),
      fov_(fov),
      update_(std::move(update_callback)),
      get_view_matrix_(std::move(get_view_matrix_callback)),
      restore_(std::move(restore_model_callback))
{
    view_matrix_ = get_view_matrix_();
    auto reshape = [&] (vec4i_t viewport) {
        std::lock_guard<std::mutex> lock(mut_);
        viewport_ = std::move(viewport);
    };
    reshape(std::move(viewport));

    events_->connect<events::mouse_scroll>([&] (int32_t delta, modifier) {
        update(vec3f_t(0.f, 0.f, 0.3f * delta), vec3f_t(0.f, 0.f, 0.f));
    });

    events->connect<events::mouse_drag>([&] (int btn, vec2f_t delta, vec2f_t, modifier mod) {
        if (btn == 1) {
            if (mod.ctrl) {
                update(vec3f_t(0.2f * delta[0], 0.2f * delta[1], 0.f), vec3f_t(0.f, 0.f, 0.f));
            } else {
                update(vec3f_t(0.f, 0.f, 0.f), vec3f_t(0.2f * delta[0], 0.2f * delta[1], 0.f));
            }
        }
    });

    events->connect<events::window_resize>(reshape);
}

mat4f_t
camera::projection_matrix(const vec2f_t& near_far) {
    float fovtan;
    float aspect;
    {
        std::lock_guard<std::mutex> lock(mut_);
        fovtan = std::tan((*fov_) * M_PI / 360.f);
        aspect = static_cast<float>(viewport_[2]) / viewport_[3];
    }

    float range = fovtan * near_far[0];
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;
    mat4f_t proj = mat4f_t::Zero();
    proj(0, 0) = (2.f * near_far[0]) / (right - left);
    proj(1, 1) = (2.f * near_far[0]) / (top - bottom);
    proj(2, 2) = -(near_far[1] + near_far[0]) / (near_far[1] - near_far[0]);
    proj(3, 2) = -1.f;
    proj(2, 3) = -(2.f * near_far[1] * near_far[0]) / (near_far[1] - near_far[0]);

    last_proj_ = proj;

    return proj;
}

mat4f_t const&
camera::last_projection_matrix() const {
    return last_proj_;
}

vec2f_t
camera::near_plane_size(const vec2f_t& near_far) const {
    std::lock_guard<std::mutex> lock(mut_);
    float fov_tan = std::tan((*fov_) * M_PI / 360.f);
    float aspect = viewport_[2] / viewport_[3];
    return 2.f * near_far[0] * vec2f_t(aspect * fov_tan, fov_tan);
}

void
camera::set_view_matrix(const mat4f_t& view_mat) {
    std::lock_guard<std::mutex> lock(mut_);
    view_matrix_ = view_mat;
    restore_(view_matrix_);
}

void
camera::store(const fs::path& p) const {
    std::ofstream out(p.string(), std::ios::out | std::ios::binary);
    if (out.good()) {
        std::lock_guard<std::mutex> lock(mut_);
        out.write((const char*) view_matrix_.data(), 16 * sizeof(float));
    }
}

void
camera::restore(const fs::path& p) {
    std::ifstream in(p.string(), std::ios::in | std::ios::binary);
    if (in.good()) {
        mat4f_t view_mat;
        in.read((char*) view_mat.data(), 16 * sizeof(float));
        set_view_matrix(view_mat);
    }
}

}  // namespace graphene
