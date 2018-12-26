#pragma once

#include "common.hpp"

namespace graphene::property {

template <typename... Args>
class section
{
public:
    section(std::string label, Args... args);

    void
    render();

protected:
    std::string label_;
    std::tuple<Args...> elements_;
};

}  // namespace graphene::property

#include "section.ipp"
