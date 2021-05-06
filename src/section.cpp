#include <section.hpp>

#include <imgui.h>

namespace graphene::property::detail {

bool
begin_section(const std::string& label) {
    return ImGui::CollapsingHeader(label.c_str());
}

} // graphene::property::detail
