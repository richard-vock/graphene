#include <button.hpp>

#include <imgui.h>

namespace graphene::property {

void
button::render()
{
    if (ImGui::Button(label_.c_str())) {
        if (func_) {
            func_();
        }
    }
}

} // graphene::property
