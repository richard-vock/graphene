#include <window.hpp>

#include <imgui.h>

namespace graphene::property::detail {

void begin_window(const std::string& label) {
    ImGui::Begin(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
}

void end_window() {
    ImGui::End();
}


} // graphene::property
