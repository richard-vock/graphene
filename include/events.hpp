#pragma once

#include "common.hpp"
#include "signal_manager.hpp"
#include "renderable.hpp"
#include <baldr/texture.hpp>

namespace graphene {

struct modifier {
    bool ctrl;
    bool alt;
    bool shift;
    bool super;
};

namespace events {

struct window_resize
{
    typedef void(signature)(vec4i_t);
};

struct mouse_click
{
    typedef void(signature)(int32_t, vec2f_t, modifier);
};

struct mouse_move
{
    typedef void(signature)(vec2f_t, vec2f_t, modifier);
};

struct mouse_drag
{
    typedef void(signature)(int32_t, vec2f_t, vec2f_t, modifier);
};

struct mouse_start_drag
{
    typedef void(signature)(int32_t, vec2f_t, modifier);
};

struct mouse_stop_drag
{
    typedef void(signature)(int32_t, vec2f_t, modifier);
};

struct mouse_scroll
{
    typedef void(signature)(int32_t, modifier);
};

struct add_object
{
    typedef void(signature)(std::string, std::shared_ptr<renderable>);
};

struct remove_object
{
    typedef void(signature)(std::string);
};

struct hide_object
{
    typedef void(signature)(std::string);
};

struct show_object
{
    typedef void(signature)(std::string);
};

struct add_image
{
    typedef void(signature)(std::string, std::shared_ptr<const baldr::texture>);
};

struct remove_image
{
    typedef void(signature)(std::string);
};

struct image_clicked
{
    typedef void(signature)(std::string, vec2i_t);
};

struct picked_vertex
{
    typedef void(signature)(std::string, uint32_t);
};

struct debug_pickray
{
    typedef void(signature)(vec3f_t, vec3f_t);
};


}  // namespace events

// clang-format off
using event_manager =
    detail::signal_manager<
        events::window_resize,
        events::mouse_click,
        events::mouse_move,
        events::mouse_drag,
        events::mouse_start_drag,
        events::mouse_stop_drag,
        events::mouse_scroll,
        events::add_object,
        events::remove_object,
        events::hide_object,
        events::show_object,
        events::add_image,
        events::remove_image,
        events::image_clicked,
        events::picked_vertex,
        events::debug_pickray
    >;
// clang-format on

}  // namespace graphene

