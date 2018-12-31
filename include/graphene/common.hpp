#pragma once

#include <string>
#include <functional>
#include <memory>
#include <tuple>
#include <array>
#include <optional>
#include <map>
#include <vector>
#include <variant>
#include <algorithm>
#include <thread>
#include <mutex>
#include <filesystem>

#include <Eigen/Dense>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "config.hpp"

namespace graphene {

#if !(defined(NDEBUG)) || defined(DEBUG) || defined(DEBUG_VERBOSE)
constexpr bool debug = true;
#ifdef DEBUG_VERBOSE
constexpr bool debug_verbose = true;
#else
constexpr bool debug_verbose = false;
#endif
#else
constexpr bool debug = false;
constexpr bool debug_verbose = false;
#endif



typedef Eigen::Vector2i vec2i_t;
typedef Eigen::Vector2f vec2f_t;
typedef Eigen::RowVector2i rvec2i_t;
typedef Eigen::RowVector2f rvec2f_t;

typedef Eigen::Vector3i vec3i_t;
typedef Eigen::Vector3f vec3f_t;
typedef Eigen::RowVector3i rvec3i_t;
typedef Eigen::RowVector3f rvec3f_t;

typedef Eigen::Vector4i vec4i_t;
typedef Eigen::Vector4f vec4f_t;
typedef Eigen::RowVector4i rvec4i_t;
typedef Eigen::RowVector4f rvec4f_t;

typedef Eigen::Matrix3f mat3f_t;
typedef Eigen::Matrix4f mat4f_t;

typedef Eigen::AlignedBox<float, 2> bbox2f_t;
typedef Eigen::AlignedBox<float, 3> bbox3f_t;

typedef Eigen::ParametrizedLine<float, 3> ray_t;

template <typename T>
struct bounds
{
    T lower;
    T upper;
};

template <typename T>
explicit bounds(T l, T b)->bounds<T>;

template <typename... Args>
inline void
pdebug(const char *format, const Args &... args)
{
    fmt::print("[D] ");
    fmt::print(format, args...);
    putchar('\n');
}

template <typename... Args>
inline void
pverbose(const char *format, const Args &... args)
{
    fmt::print("[V] ");
    fmt::print(format, args...);
    putchar('\n');
}

template <typename... Args>
inline void
perror(const char *format, const Args &... args)
{
    fmt::print("[E] ");
    fmt::print(format, args...);
    putchar('\n');
}

template <typename... Args>
inline void
fail(const char *format, const Args &... args)
{
    perror(format, args...);
    std::terminate();
}

template <typename... Args>
inline void
terminate_unless(bool condition, const char* msg, const Args &... args)  {
    if (!condition) {
        fail(msg, args...);
    }
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace graphene
