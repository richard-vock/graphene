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

    choose_file(choose_file const& other) {
        label_ = other.label_;
        func_ = other.func_;
        path_ = other.path_;
        chosen_ = other.chosen_;
        list_filter_ = other.list_filter_;
        choose_filter_ = other.choose_filter_;
    }

    choose_file(choose_file && other) {
        label_ = std::move(other.label_);
        func_ = std::move(other.func_);
        path_ = std::move(other.path_);
        chosen_ = std::move(other.chosen_);
        list_filter_ = std::move(other.list_filter_);
        choose_filter_ = std::move(other.choose_filter_);
    }

    choose_file&
    operator =(choose_file const& other) {
        label_ = other.label_;
        func_ = other.func_;
        path_ = other.path_;
        chosen_ = other.chosen_;
        list_filter_ = other.list_filter_;
        choose_filter_ = other.choose_filter_;

        return *this;
    }

    choose_file&
    operator =(choose_file && other) {
        label_ = std::move(other.label_);
        func_ = std::move(other.func_);
        path_ = std::move(other.path_);
        chosen_ = std::move(other.chosen_);
        list_filter_ = std::move(other.list_filter_);
        choose_filter_ = std::move(other.choose_filter_);

        return *this;
    }

    void render();

protected:
    std::string label_;
    std::function<void(std::filesystem::path)> func_;
    std::filesystem::path path_;
    bool chosen_;
    std::function<bool(std::filesystem::path)> list_filter_;
    std::function<bool(std::filesystem::path)> choose_filter_;
};

}  // namespace property

}  // namespace graphene

#include "choose_file.ipp"
