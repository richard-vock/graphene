#pragma once

#include "common.hpp"
#include "events.hpp"


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

protected:
    std::unique_ptr<impl> impl_;
};

}  // namespace graphene

#include "application.ipp"
