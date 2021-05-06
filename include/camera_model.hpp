#pragma once

#include "common.hpp"

namespace graphene {

struct looking_at {
    vec3f_t position;
    vec3f_t look_at;
};

struct looking_towards {
    vec3f_t position;
    vec3f_t direction;
};

typedef std::variant<looking_at, looking_towards> view_t;

template <typename Model>
struct camera_traits {
    static std::shared_ptr<Model>
    init(std::variant<looking_at, looking_towards> view);

    static void
    update(std::shared_ptr<Model> model, vec3f_t translational, vec3f_t rotational);

    static mat4f_t
    view_matrix(const std::shared_ptr<Model> model);

    static void
    restore(std::shared_ptr<Model> model, mat4f_t const& p);
};

} // graphene
