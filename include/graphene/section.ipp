namespace graphene::property {

namespace detail {

bool begin_section(const std::string& label);

} // detail

template <typename... Args>
inline
section<Args...>::section(std::string label, Args... args)
    : label_(std::move(label)), elements_(std::move(args)...)
{}

template <typename... Args>
inline void
section<Args...>::render()
{
    if (!detail::begin_section(label_)) {
        return;
    }
    std::apply([](auto&... el) { (el.render(), ...); }, elements_);
}

}  // namespace graphene::property
