#include <rgb.hpp>

#include <imgui.h>

namespace graphene::property {

rgb::rgb(std::string label, shared<vec3f_t> value)
    : label_(std::move(label)), value_(value)
{}

void
rgb::render()
{
    vec3f_t tmp = value_;
    if (ImGui::ColorEdit3(label_.c_str(), tmp.data())) {
        value_ = tmp;
    }
}

} // graphene::property
