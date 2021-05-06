namespace graphene::property {

namespace detail {

void begin_window(const std::string& label);

void end_window();

} // detail

template <typename... Args>
inline
window<Args...>::window(std::string label, Args... args) : label_(std::move(label)), elements_(std::move(args)...) {}

template <typename... Args>
inline void
window<Args...>::render() {
    detail::begin_window(label_);
    std::apply([](auto& ...el){ (el.render(), ...);} , elements_);
    detail::end_window();
}

} // graphene::property
