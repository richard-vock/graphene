#pragma once

#include "common.hpp"

namespace graphene::property {

class text
{
public:
    text(const char* txt);

    text(std::string_view txt);

    text(const std::string& txt);

    template <typename F>
    text(F func);

    void render();

protected:
    std::string text_;
    std::function<std::string()> func_;
};

}  // namespace graphene::property

#include "text.ipp"
