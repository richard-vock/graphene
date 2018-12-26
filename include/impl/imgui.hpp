#pragma once

#include <string>
#include <imgui.h>
#include <graphene/common.hpp>
#include <graphene/events.hpp>

struct GLFWwindow;

namespace graphene::imgui {

IMGUI_IMPL_API bool init_application(GLFWwindow* window, const std::string& font_path, float font_size, std::shared_ptr<event_manager> events);

IMGUI_IMPL_API void shutdown_application();

IMGUI_IMPL_API void init_frame();

IMGUI_IMPL_API void draw_frame();

}  // namespace graphene::imgui
