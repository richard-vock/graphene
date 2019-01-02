#pragma once

#include "common.hpp"
#include "signal_manager.hpp"
#include "renderable.hpp"

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
    typedef void(signature)(std::string, std::shared_ptr<const renderable>);
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
        events::show_object
    >;
// clang-format on

}  // namespace graphene

