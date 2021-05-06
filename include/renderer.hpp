#pragma once

#include <Eigen/Dense>
#include "shared.hpp"
#include "events.hpp"

namespace baldr {

class texture;

}

namespace graphene {

class camera;

namespace detail {

class renderer
{
public:
    struct parameters {
        shared<bool> show_normals = false;
        shared<float> splat_radius = 1.0f;

        shared<Eigen::Vector4f> background_inner = Eigen::Vector4f(0.15f, 0.15f, 0.15f, 1.f);
        shared<Eigen::Vector4f> background_outer = Eigen::Vector4f(0.05f, 0.05f, 0.05f, 1.f);

        shared<bool> tonemapping_enabled = true;
        shared<bool> rebuild_lut = true;
        shared<float> film_slope = 0.78f;
        shared<float> film_toe = 0.45f;
        shared<float> film_shoulder = 0.25f;
        shared<float> film_black_clip = 0.0f;
        shared<float> film_white_clip = 0.04f;

        shared<float> pick_radius = 0.05f;

        shared<bool> render_depth_buffer = false;
        shared<bool> debug = false;
    };

public:
    renderer(std::shared_ptr<event_manager> events, std::shared_ptr<camera> cam, const parameters& params);

    virtual ~renderer();

    void
    render(std::shared_ptr<baldr::texture> render_to_texture = nullptr,
           std::shared_ptr<baldr::texture> render_depth_to_texture = nullptr);

    void
    render(Eigen::Matrix4f const& view_matrix,
           Eigen::Matrix4f const& proj_matrix,
           std::shared_ptr<baldr::texture> render_to_texture = nullptr,
           std::shared_ptr<baldr::texture> render_depth_to_texture = nullptr);

protected:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace detail

}  // namespace graphene
