#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class rgba
{
public:
    rgba(std::string label, shared<vec4f_t> value);

    template <typename F>
    rgba(std::string label, shared<vec4f_t> value, F func);

    template <typename F>
    rgba(std::string label, vec4f_t default_value, F func);

    void render();

protected:
    std::string label_;
    tracked<vec4f_t> value_;
};

}  // namespace graphene::property

#include "rgba.ipp"
