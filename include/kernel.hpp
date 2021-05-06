#pragma once

#include <string>
#include <baldr/shader_program.hpp>
#include <baldr/shader_pipeline.hpp>

#include "config.hpp"

namespace graphene::detail {

struct kernel {
    kernel(std::string const& name);

    void
    sync(GLbitfield barriers = GL_ALL_BARRIER_BITS) noexcept;

    void
    launch(GLuint groups_x, GLuint groups_y = 1, GLuint groups_z = 1) noexcept;

    void
    launch1(GLuint width) noexcept;

    void
    launch1(GLuint width, GLuint work_size_0) noexcept;

    void
    launch2(GLuint width, GLuint height) noexcept;

    void
    launch2(GLuint width, GLuint height, GLuint work_size_0, GLuint work_size_1) noexcept;

    void
    launch3(GLuint width, GLuint height, GLuint depth) noexcept;

    void
    launch3(GLuint width, GLuint height, GLuint depth, GLuint work_size_0, GLuint work_size_1, GLuint work_size_2) noexcept;

    void
    operator()(GLuint groups_x, GLuint groups_y = 1, GLuint groups_z = 1) noexcept;

    [[ nodiscard ]]
    const baldr::shader_uniform&
    uniform(const std::string& name) const noexcept;

    [[ nodiscard ]]
    const baldr::sampler_unit&
    sampler(const std::string& name) const noexcept;

    [[ nodiscard ]]
    const baldr::image_unit&
    image(const std::string& name) const noexcept;

    [[ nodiscard ]]
    const baldr::shader_ssbo&
    ssbo(const std::string& name) const noexcept;

    std::shared_ptr<baldr::shader_program> program;
    std::shared_ptr<baldr::shader_pipeline> pipeline;
};

}  // namespace graphene::detail

#include "kernel.ipp"
