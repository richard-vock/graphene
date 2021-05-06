#pragma once

#include "camera_model.hpp"

namespace graphene {

struct orbit_camera_model;

template <>
struct camera_traits<orbit_camera_model> {
    static std::shared_ptr<orbit_camera_model>
    init(std::variant<looking_at, looking_towards> view);

    static void
    update(std::shared_ptr<orbit_camera_model> model, vec3f_t translational, vec3f_t rotational);

    static mat4f_t
    view_matrix(std::shared_ptr<const orbit_camera_model> model);

    static void
    restore(std::shared_ptr<orbit_camera_model> model, mat4f_t const& p);
};

} // graphene
