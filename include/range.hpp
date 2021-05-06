#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class range
{
public:
    range(std::string label, shared<float> value, bounds<float> range);

    template <typename F>
    range(std::string label, shared<float> value, bounds<float> range, F func);

    template <typename F>
    range(std::string label, float default_value, bounds<float> range, F func);

    void render();

protected:
    std::string label_;
    bounds<float> range_;
    tracked<float> value_;
};

}  // namespace graphene::property

#include "range.ipp"
