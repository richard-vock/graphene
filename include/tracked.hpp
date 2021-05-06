#pragma once

#include <functional>
#include <memory>
#include <variant>

#include "shared.hpp"

namespace graphene::property {

template <typename T>
struct tracked
{
public:
    tracked(shared<T> value);

    template <typename F>
    tracked(shared<T> value, F func);

    template <typename F>
    tracked(T default_value, F func);

    operator T() const;

    tracked& operator=(T value);

protected:
    shared<T> value_;
    std::function<void(T)> func_;
};

}  // namespace graphene::property

#include "tracked.ipp"
