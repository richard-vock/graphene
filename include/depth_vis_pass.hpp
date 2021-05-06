#pragma once

#include <memory>
#include <Eigen/Dense>

namespace baldr {

class texture;

} // baldr

namespace graphene::detail {

class depth_vis_pass {
public:
    depth_vis_pass();

    ~depth_vis_pass();

    void
    render(std::shared_ptr<baldr::texture> depth_buffer,
           Eigen::Matrix4f const& projection_matrix,
           std::shared_ptr<baldr::texture> render_to_texture = nullptr);

protected:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // graphene::detail
