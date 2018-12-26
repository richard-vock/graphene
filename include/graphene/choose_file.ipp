namespace graphene::property {

namespace detail {

inline std::filesystem::path
canonical_path(std::filesystem::path p)
{
    return std::filesystem::canonical(std::filesystem::is_directory(p) ? p : p.parent_path());
}

}  // namespace detail

template <typename Func, typename... Filter>
inline choose_file::choose_file(const std::string label, Func&& func,
                                Filter&&... filter)
    : label_(std::move(label)),
      func_(func),
      path_(detail::canonical_path(std::filesystem::current_path())),
      chosen_(false)
{
    list_filter_ = [=](const std::filesystem::path& p) {
        auto tup = std::make_tuple(filter...);
        return std::apply([&](auto&&... filt) { return (filt.show(p) && ...); },
                          tup);
    };
    choose_filter_ = [=](const std::filesystem::path& p) {
        auto tup = std::make_tuple(filter...);
        return std::apply(
            [&](auto&&... filt) { return (filt.accept(p) && ...); }, tup);
    };
}

}  // namespace graphene::property
