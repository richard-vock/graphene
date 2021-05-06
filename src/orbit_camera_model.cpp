#include <orbit_camera_model.hpp>

namespace fs = std::filesystem;

namespace graphene {

struct orbit_camera_model {
    vec3f_t look_at;
    float  phi;
    float  theta;
    float  psi;
    float  radius;
    mat4f_t view_matrix;
};

std::shared_ptr<orbit_camera_model>
camera_traits<orbit_camera_model>::init(std::variant<looking_at, looking_towards> view) {
    std::shared_ptr<orbit_camera_model> model(new orbit_camera_model({
        .look_at = vec3f_t::Zero(),
        .phi = 0.f,
        .theta = 0.f,
        .psi = 0.f,
        .radius = 20.f,
        .view_matrix = mat4f_t::Identity()
    }));

    auto && [pos, look_at] = std::visit(overloaded {
        [](looking_at      v) -> std::pair<vec3f_t, vec3f_t> { auto && [pos, lat] = v; return { pos, lat }; },
        [](looking_towards v) -> std::pair<vec3f_t, vec3f_t> { auto && [pos, dir] = v; return { pos, pos+dir }; },
    }, view);

    pos -= look_at;
    model->look_at = look_at;

    model->radius = pos.norm();
    pos /= model->radius;
    model->theta = -std::asin(pos[1]);
    pos[1] = 0.f;
    float len = pos.norm();
    if (len < Eigen::NumTraits<float>::dummy_precision()) {
        model->phi = 0.f;
    } else {
        pos /= len;
        model->phi = std::atan2(pos[0], pos[2]);
        if (model->phi < 0.f) {
            model->phi += 2.f * static_cast<float>(M_PI);
        }
    }

    camera_traits<orbit_camera_model>::update(model, vec3f_t::Zero(), vec3f_t::Zero());

    return model;
}

void
camera_traits<orbit_camera_model>::update(std::shared_ptr<orbit_camera_model> model, vec3f_t translational, vec3f_t rotational) {
    if (translational.head(2).squaredNorm() > Eigen::NumTraits<float>::epsilon()) {
        vec3f_t delta = model->view_matrix.topLeftCorner<3,3>().transpose() * vec3f_t(-translational[0], translational[1], 0.f);
        model->look_at += 0.024f * delta;
    }

    if (fabs(translational[2]) > Eigen::NumTraits<float>::epsilon()) {
        model->radius = std::max((model->radius - translational[2]), Eigen::NumTraits<float>::epsilon());
    }

    if (rotational.squaredNorm() > Eigen::NumTraits<float>::epsilon()) {
        constexpr float pi2 = 2.f * M_PI;
        constexpr float pdiv2 = 0.5f * M_PI;

        model->phi   -= 0.01f * rotational[0];
        model->theta -= 0.01f * rotational[1];
        model->psi   -= 0.01f * rotational[2];

        // correctly bound values
        while (model->phi <    0) model->phi += pi2;
        while (model->phi >= pi2) model->phi -= pi2;
        if (model->theta < -pdiv2) model->theta = -pdiv2;
        if (model->theta >  pdiv2) model->theta =  pdiv2;
        if (model->psi < -pdiv2) model->psi = -pdiv2;
        if (model->psi >  pdiv2) model->psi =  pdiv2;
    }

    Eigen::Affine3f t;
	t = Eigen::Translation<float,3>(-model->radius * vec3f_t::UnitZ())
	  * Eigen::AngleAxisf(-model->theta, vec3f_t::UnitX())
	  * Eigen::AngleAxisf(-model->phi  , vec3f_t::UnitY())
	  //* Eigen::AngleAxisf(-psi_  , vec3f_t::UnitZ())
	  * Eigen::Translation<float,3>(-model->look_at);
	model->view_matrix = t.matrix();
}

mat4f_t
camera_traits<orbit_camera_model>::view_matrix(std::shared_ptr<const orbit_camera_model> model) {
    return model->view_matrix;
}

void
camera_traits<orbit_camera_model>::restore(std::shared_ptr<orbit_camera_model> model, mat4f_t const& view_mat) {
    vec3f_t pos = -view_mat.topLeftCorner<3,3>().transpose() *
                   view_mat.topRightCorner<3,1>();
    vec3f_t fwd = -view_mat.block<1, 3>(2, 0).transpose();
    *model = *camera_traits<orbit_camera_model>::init(looking_towards{pos, fwd});
}

} // graphene
