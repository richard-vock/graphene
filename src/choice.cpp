#include <graphene/choice.hpp>

#include <imgui.h>

namespace graphene::property::detail {

void
render_line() {
    ImGui::SameLine();
}

bool
render_radio_button(const std::string& label, bool state) {
    return ImGui::RadioButton(label.c_str(), state);
}

} // graphene::property::detail
