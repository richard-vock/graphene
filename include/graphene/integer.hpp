#pragma once

#include "common.hpp"
#include "tracked.hpp"

namespace graphene::property {

class integer
{
public:
    integer(std::string label, shared<int> value, int step = 0);

    template <typename F>
    integer(std::string label, shared<int> value, F func);

    template <typename F>
    integer(std::string label, shared<int> value, int step, F func);

    template <typename F>
    integer(std::string label, int default_value, F func);

    template <typename F>
    integer(std::string label, int default_value, int step, F func);

    void render();

protected:
    std::string label_;
    tracked<int> value_;
    int step_;
};

}  // namespace graphene::property

#include "integer.ipp"
