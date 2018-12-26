#pragma once

#include "camera_model.hpp"
#include "events.hpp"

namespace graphene {

class camera
{
public:
    template <typename Model>
    static std::shared_ptr<camera>
    create(view_t view, std::shared_ptr<event_manager> events, vec4i_t viewport,
           float fov, float near, float far);

    vec3f_t
    position() const;

    vec3f_t
    forward() const;

    vec3f_t
    right() const;

    vec3f_t
    up() const;

    float
    near() const;

    float
    far() const;

    float
    fov() const;

    const vec4i_t&
    viewport() const;

    mat4f_t
    view_matrix() const;

    mat3f_t
    normal_matrix() const;

    const mat4f_t&
    projection_matrix() const;

    bool
    projection_matrix_changed();

    void
    update(vec3f_t translational, vec3f_t rotational);

    Eigen::ParametrizedLine<float, 3>
    pick_ray(const vec2i_t& px);

protected:
    camera(std::shared_ptr<event_manager> events, vec4i_t viewport, float fov,
           float near, float far, std::function<void(vec3f_t, vec3f_t)> update,
           std::function<mat4f_t()> get_view_matrix);

protected:
    std::shared_ptr<event_manager> events_;
    vec4i_t viewport_;
    float fov_;
    float near_;
    float far_;
    std::function<void(vec3f_t, vec3f_t)> update_;
    std::function<mat4f_t()> get_view_matrix_;
    mat4f_t view_matrix_;
    mat4f_t math_to_opengl_;
    mat4f_t opengl_to_math_;
    mat4f_t projection_matrix_;
    bool projection_matrix_changed_;
    mutable std::mutex mut_;
};

}  // namespace graphene

#include "camera.ipp"
