#pragma once

#include "common.hpp"
#include "text.hpp"
#include "tracked.hpp"

namespace graphene {

template <typename T>
struct active
{
    active(T v) : value(v) {}

    T value;
};

template <typename T>
struct option
{
    option(const std::string& l, T v) : label(l), value(v) {}

    std::string label;
    T value;
};

namespace property {

template <typename T>
class choice
{
public:
    template <typename... Ts>
    choice(std::string label, shared<T> value, option<T> possible,
           Ts&&... more);

    template <typename F, typename... Ts>
    choice(std::string label, F func,
           std::variant<option<T>, active<option<T>>> possible, Ts&&... more);

    void render();

protected:
    std::string label_;
    tracked<T> value_;
    text text_;
    std::vector<std::string> labels_;
    std::vector<T> possible_;
    uint32_t active_;
};

}  // namespace property
}  // namespace graphene

#include "choice.ipp"
