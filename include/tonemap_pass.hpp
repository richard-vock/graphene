#pragma once

#include <memory>

#include "common.hpp"
#include "shared.hpp"

namespace baldr {

class texture;

} // baldr

namespace graphene::detail {

class tonemap_pass {
public:
    tonemap_pass(shared<bool> enabled,
                 shared<bool> needs_rebuild,
                 shared<float> film_slope,
                 shared<float> film_shoulder,
                 shared<float> film_toe,
                 shared<float> film_black_clip,
                 shared<float> film_white_clip);

    ~tonemap_pass();

    void
    render(std::shared_ptr<baldr::texture> linear_color,
           std::shared_ptr<baldr::texture> render_to_texture = nullptr);

protected:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // graphene::detail
