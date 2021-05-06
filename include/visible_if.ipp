namespace graphene::property {

template <typename... Args>
inline
visible_if<Args...>::visible_if(shared<bool> is_visible, Args... args) : visible_if<Args...>([is_visible] () -> bool { return is_visible; }, std::move(args)...) {}

template <typename... Args>
template <typename Func>
inline
visible_if<Args...>::visible_if(Func is_visible, Args... args) : is_visible_(is_visible), elements_(std::move(args)...) {}

template <typename... Args>
inline void
visible_if<Args...>::render() {
    if (is_visible_()) {
        std::apply([](auto& ...el){ (el.render(), ...);} , elements_);
    }
}

} // graphene::property
