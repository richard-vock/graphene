#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class boolean
{
public:
    boolean(std::string label, shared<bool> value);

    template <typename F>
    boolean(std::string label, shared<bool> value, F func);

    template <typename F>
    boolean(std::string label, bool default_value, F func);

    void render();

protected:
    std::string label_;
    tracked<bool> value_;
};

}  // namespace graphene::property

#include "boolean.ipp"
