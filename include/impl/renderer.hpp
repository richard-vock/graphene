#pragma once

#include <baldr/baldr.hpp>
#include <graphene/camera.hpp>
#include <graphene/common.hpp>
#include <graphene/events.hpp>
#include <graphene/shared.hpp>

#include "point_visibility.hpp"
#include "texture_pyramid.hpp"

namespace graphene::detail {

class renderer
{
public:
    struct parameters {
        shared<float> occlusion_threshold = 0.95f;
        shared<float> sigma_depth = 0.03;
        shared<bool> fill = true;
        shared<bool> skip_bilateral_filter = false;
        shared<bool> show_normals = false;
        shared<bool> debug = false;
        shared<float> debug_float = 0.f;
        shared<float> point_scale = 0.01f;
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

    std::shared_ptr<baldr::shader_program> gbuffer_shader_;
    std::shared_ptr<baldr::fullscreen_pass> gbuffer_pass_;
    std::shared_ptr<baldr::texture> gbuffer_;

    std::shared_ptr<baldr::texture> depth_;
    std::shared_ptr<baldr::shader_program> bilateral_filter_shader_;
    std::shared_ptr<baldr::fullscreen_pass> bilateral_filter_pass_;

    std::shared_ptr<baldr::shader_program> normal_shader_;
    std::shared_ptr<baldr::fullscreen_pass> normal_shader_pass_;

    std::shared_ptr<baldr::shader_program> visibility_shader_;
    std::shared_ptr<baldr::fullscreen_pass> visibility_pass_;
    std::shared_ptr<baldr::texture> visibility_map_;
    std::shared_ptr<baldr::shader_program> build_pyramid_shader_;
    std::shared_ptr<texture_pyramid> pyramid_;

    std::mutex data_mutex_;
    std::map<std::string, render_data> objects_;
    shared<vec4f_t> clear_color_;

    // testing
    std::shared_ptr<baldr::texture> inp_;
    std::shared_ptr<baldr::texture> outp_;
    std::shared_ptr<baldr::shader_program> filt_shader_;
    std::shared_ptr<baldr::fullscreen_pass> filt_pass_;

    parameters params_;
};

}  // namespace graphene::detail
