#pragma once

#include "common.hpp"
#include "shared.hpp"

namespace graphene::property {

template <typename... Args>
class visible_if
{
public:
    visible_if(shared<bool> is_visible, Args... args);

    template <typename Func>
    visible_if(Func visible_if, Args... args);

    void render();

protected:
    std::function<bool()> is_visible_;
    std::tuple<Args...> elements_;
};

}  // namespace graphene::property

#include "visible_if.ipp"
