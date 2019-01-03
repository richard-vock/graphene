#pragma once

#include <baldr/baldr.hpp>
#include <graphene/camera.hpp>
#include <graphene/common.hpp>
#include <graphene/events.hpp>
#include <graphene/shared.hpp>

#include "point_visibility.hpp"

namespace graphene::detail {

class renderer
{
public:
    struct parameters {
        shared<float> occlusion_threshold = 1.f;
        shared<bool> fill = false;
        shared<bool> show_normals = false;
        shared<bool> debug = false;
    };

protected:
    struct render_data
    {
        bool hidden;
        mat4f_t transform;
        bbox3f_t bbox;
        GLenum mode;
        bool shaded;
        uint32_t vertex_count;
        std::shared_ptr<baldr::texture> texture;
        std::unique_ptr<baldr::data_buffer> vbo;
        std::shared_ptr<baldr::data_buffer> ibo;
        std::unique_ptr<baldr::vertex_array> vao;
    };

public:
    renderer(std::shared_ptr<event_manager> events, std::shared_ptr<camera> cam, const parameters& params);

    virtual ~renderer();

    void
    init();

    void
    render();

protected:
    std::shared_ptr<event_manager> events_;
    std::shared_ptr<camera> cam_;

    std::shared_ptr<baldr::shader_program> geometry_vs_;
    std::shared_ptr<baldr::shader_program> geometry_fs_;
    std::shared_ptr<baldr::render_pass> geometry_pass_;

    std::shared_ptr<baldr::texture> geometry_depth_;
    std::shared_ptr<baldr::shader_program> render_depth_shader_;
    std::shared_ptr<baldr::fullscreen_pass> render_depth_pass_;

    std::shared_ptr<point_visibility> point_visibility_;

    std::mutex data_mutex_;
    std::map<std::string, render_data> objects_;
    shared<vec4f_t> clear_color_;

    parameters params_;
};

}  // namespace graphene::detail
