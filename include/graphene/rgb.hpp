#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class rgb
{
public:
    rgb(std::string label, shared<vec3f_t> value);

    template <typename F>
    rgb(std::string label, shared<vec3f_t> value, F func);

    template <typename F>
    rgb(std::string label, vec3f_t default_value, F func);

    void render();

protected:
    std::string label_;
    tracked<vec3f_t> value_;
};

}  // namespace graphene::property

#include "rgb.ipp"
