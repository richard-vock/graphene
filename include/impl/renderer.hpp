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
        std::unique_ptr<baldr::data_buffer> ibo;
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
    std::shared_ptr<baldr::shader_pipeline> pipeline_;

    std::shared_ptr<baldr::texture> gbuffer_depth_;
    std::shared_ptr<baldr::shader_program> render_depth_shader_;
    std::shared_ptr<baldr::fullscreen_pass> render_depth_;

    std::shared_ptr<baldr::texture> visibility_mask_;
    std::shared_ptr<baldr::shader_program> visibility_shader_;
    std::shared_ptr<baldr::fullscreen_pass> visibility_pass_;

    std::mutex data_mutex_;
    std::map<std::string, render_data> objects_;
    vec3f_t clear_color_;

    shared<float> occlusion_threshold_;
};

}  // namespace graphene::detail
