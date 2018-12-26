#include <graphene/real.hpp>

#include <imgui.h>

namespace graphene::property {

real::real(std::string label, shared<float> value, uint32_t precision,
           float step)
    : label_(std::move(label)),
      value_(value),
      precision_(precision),
      step_(step)
{}

void
real::render()
{
    float tmp = value_;

    std::string format = fmt::format("%.{}f", precision_);
    if (ImGui::InputFloat(label_.c_str(), &tmp, step_, 0.f, format.c_str())) {
        value_ = tmp;
    }
}

}  // namespace graphene::property
