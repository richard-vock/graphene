#include <boolean.hpp>

#include <imgui.h>

namespace graphene::property {

boolean::boolean(std::string label, shared<bool> value)
    : label_(std::move(label)), value_(value)
{}

void
boolean::render()
{
    bool tmp = value_;
    if (ImGui::Checkbox(label_.c_str(), &tmp)) {
        value_ = tmp;
    }
}

} // graphene::property
