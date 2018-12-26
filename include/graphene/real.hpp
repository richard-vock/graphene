#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class real
{
public:
    real(std::string label, shared<float> value, uint32_t precision, float step = 0.f);

    template <typename F>
    real(std::string label, shared<float> value, uint32_t precision, F func);

    template <typename F>
    real(std::string label, shared<float> value, uint32_t precision, float step, F func);

    template <typename F>
    real(std::string label, float default_value, uint32_t precision, F func);

    template <typename F>
    real(std::string label, float default_value, uint32_t precision, float step, F func);

    void render();

protected:
    std::string label_;
    tracked<float> value_;
    uint32_t precision_;
    float step_;
};

}  // namespace graphene::property

#include "real.ipp"
