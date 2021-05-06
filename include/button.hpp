#pragma once

#include "common.hpp"

namespace graphene::property {

class button
{
public:
    template <typename Func>
    button(const std::string& label, Func func);

    void
    render();

protected:
    std::string label_;
    std::function<void()> func_;
};

} // graphene::property

#include "button.ipp"
