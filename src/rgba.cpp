#include <rgba.hpp>

#include <imgui.h>

namespace graphene::property {

rgba::rgba(std::string label, shared<vec4f_t> value)
    : label_(std::move(label)), value_(value)
{}

void
rgba::render()
{
    vec4f_t tmp = value_;
    if (ImGui::ColorEdit4(label_.c_str(), tmp.data())) {
        value_ = tmp;
    }
}

} // graphene::property
