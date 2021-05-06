#include <integer.hpp>

#include <imgui.h>

namespace graphene::property {

integer::integer(std::string label, shared<int> value,
           int step)
    : label_(std::move(label)),
      value_(value),
      step_(step)
{}

void
integer::render()
{
    int tmp = value_;

    if (ImGui::InputInt(label_.c_str(), &tmp, step_)) {
        value_ = tmp;
    }
}

}  // namespace graphene::property
