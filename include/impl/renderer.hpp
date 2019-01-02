#pragma once

#include <baldr/baldr.hpp>
#include <graphene/camera.hpp>
#include <graphene/common.hpp>
#include <graphene/events.hpp>
#include <graphene/shared.hpp>

namespace graphene::detail {

class renderer
{
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
    renderer(std::shared_ptr<event_manager> events, std::shared_ptr<camera> cam, shared<float> occlusion_threshold);

    virtual ~renderer();

    void
    init();

    void
    render();

protected:
    std::shared_ptr<event_manager> events_;
    std::shared_ptr<camera> cam_;

    std::shared_ptr<baldr::shader_program> gbuffer_vs_;
    std::shared_ptr<baldr::shader_program> gbuffer_fs_;
    std::shared_ptr<baldr::render_pass> gbuffer_pass_;

    std::shared_ptr<baldr::texture> gbuffer_depth_;
    std::shared_ptr<baldr::shader_program> render_depth_shader_;
    std::shared_ptr<baldr::fullscreen_pass> render_depth_pass_;

    std::shared_ptr<baldr::texture> visibility_map_;
    std::shared_ptr<baldr::texture> visibility_map_pp_;
    std::shared_ptr<baldr::shader_program> visibility_shader_;
    std::shared_ptr<baldr::shader_program> anisotropic_fill_shader_;
    std::shared_ptr<baldr::fullscreen_pass> visibility_pass_;
    std::shared_ptr<baldr::fullscreen_pass> anisotropic_fill_pass_;

    std::mutex data_mutex_;
    std::map<std::string, render_data> objects_;
    shared<vec4f_t> clear_color_;

    shared<float> occlusion_threshold_;
};

}  // namespace graphene::detail
