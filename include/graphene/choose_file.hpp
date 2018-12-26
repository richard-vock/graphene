#pragma once

#include "common.hpp"

namespace graphene {

namespace filter {

struct open
{
    open();

    bool show(const std::filesystem::path& p);

    bool accept(const std::filesystem::path& p);
};

struct save
{
    save();

    bool show(const std::filesystem::path& p);

    bool accept(const std::filesystem::path& p);
};

struct file
{
    file();

    bool show(const std::filesystem::path& p);

    bool accept(const std::filesystem::path& p);
};

struct directory
{
    directory();

    bool show(const std::filesystem::path& p);

    bool accept(const std::filesystem::path& p);
};

struct with_extension
{
    with_extension(std::string ext);

    bool show(const std::filesystem::path& p);

    bool accept(const std::filesystem::path& p);

    std::string ext_;
};

}  // namespace filter

namespace property {

class choose_file
{
public:
    template <typename Func, typename... Filter>
    choose_file(const std::string label, Func&& func, Filter&&... filter);

    void render();

protected:
    std::string label_;
    std::function<void(const std::filesystem::path&)> func_;
    std::filesystem::path path_;
    bool chosen_;
    std::function<bool(const std::filesystem::path&)> list_filter_;
    std::function<bool(const std::filesystem::path&)> choose_filter_;
};

}  // namespace property

}  // namespace graphene

#include "choose_file.ipp"
