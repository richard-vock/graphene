#include <range.hpp>

#include <imgui.h>

namespace graphene::property {

range::range(std::string label, shared<float> value, bounds<float> range)
    : label_(std::move(label)),
      range_(std::move(range)),
      value_(value)
{}

void
range::render()
{
    float tmp = value_;
    if (ImGui::SliderFloat(label_.c_str(), &tmp, range_.lower,
                           range_.upper)) {
        value_ = tmp;
    }
}

} // graphene::property
