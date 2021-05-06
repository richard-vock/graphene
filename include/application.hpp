#pragma once

#include "common.hpp"
#include "events.hpp"
#include "camera.hpp"


namespace graphene {

struct font_spec {
    std::filesystem::path path;
    float size;
};

class application
{
public:
    struct parameters {
        vec2i_t window_size;
        std::string window_title;
        bool vsync;
        font_spec font;
    };

    struct impl;

public:
    application(const parameters& params);

    virtual ~application();

    template <typename Init, typename... Windows>
    void
    run(Init&& init, Windows&&... windows);

    vec2i_t
    window_size() const;

    float
    framerate() const;

    std::shared_ptr<event_manager>
    events();

    std::shared_ptr<camera>
    cam();

    void
    render_to_texture(mat4f_t const& view_matrix,
                      mat4f_t const& proj_matrix,
                      std::shared_ptr<baldr::texture> color_output = nullptr,
                      std::shared_ptr<baldr::texture> depth_output = nullptr);

    void
    render_to_texture(std::shared_ptr<baldr::texture> color_output = nullptr,
                      std::shared_ptr<baldr::texture> depth_output = nullptr);

protected:
    std::unique_ptr<impl> impl_;
};

}  // namespace graphene

#include "application.ipp"
