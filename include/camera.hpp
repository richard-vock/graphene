#pragma once

#include <filesystem>

#include "camera_model.hpp"
#include "events.hpp"
#include "shared.hpp"

namespace graphene {

class camera
{
public:
    template <typename Model>
    static std::shared_ptr<camera>
    create(view_t view, std::shared_ptr<event_manager> events,
           vec4i_t viewport,
           shared<float> fov);

    vec3f_t
    position() const;

    vec3f_t
    forward() const;

    vec3f_t
    right() const;

    vec3f_t
    up() const;

    float
    fov() const;

    float
    near() const;

    float
    far() const;

    const vec4i_t&
    viewport() const;

    mat4f_t
    view_matrix() const;

    mat3f_t
    normal_matrix() const;

    mat4f_t
    projection_matrix(const vec2f_t& near_far);

    mat4f_t const&
    last_projection_matrix() const;

    void
    update(vec3f_t translational, vec3f_t rotational);

    ray_t
    pick_ray(const vec2i_t& px, const vec2f_t& near_far);

    vec2f_t
    near_plane_size(const vec2f_t& near_far) const;

    void
    set_view_matrix(const mat4f_t& view_mat);

    void
    store(const std::filesystem::path& p) const;

    void
    restore(const std::filesystem::path& p);

protected:
    camera(std::shared_ptr<event_manager> events, vec4i_t viewport, shared<float> fov,
           std::function<void(vec3f_t, vec3f_t)> update,
           std::function<mat4f_t()> get_view_matrix,
           std::function<void(mat4f_t const&)> restore_model_callback);

protected:
    std::shared_ptr<event_manager> events_;
    vec4i_t viewport_;
    shared<float> fov_;
    std::function<void(vec3f_t, vec3f_t)> update_;
    std::function<mat4f_t()> get_view_matrix_;
    std::function<void(mat4f_t const&)> restore_;
    mat4f_t view_matrix_;
    mat4f_t last_proj_;
    mutable std::mutex mut_;
};

}  // namespace graphene

#include "camera.ipp"
