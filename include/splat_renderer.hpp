#pragma once

#include <map>
#include <memory>

#include <Eigen/Dense>

#include "shared.hpp"

namespace baldr {

class texture;
class data_buffer;
class vertex_array;

} // baldr

namespace graphene {

class renderable;

namespace detail {

class splat_renderer {
public:
    splat_renderer(shared<float> splat_radius,
                   shared<Eigen::Vector4f> clear_color_inner,
                   shared<Eigen::Vector4f> clear_color_outer);

    ~splat_renderer();

    void
    render(std::map<std::string, std::shared_ptr<renderable>> const& objects,
           std::shared_ptr<baldr::texture> linear_output,
           std::shared_ptr<baldr::texture> depth_buffer,
           Eigen::Matrix4f const& view_mat,
           Eigen::Matrix4f const& proj_mat,
           Eigen::Vector4i const& viewport);

    void bind_vertex_array(std::shared_ptr<baldr::vertex_array> vertex_array_object,
                           std::shared_ptr<baldr::data_buffer> vertex_buffer_object);

protected:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // detail

} // graphene
