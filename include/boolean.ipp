namespace graphene::property {

template <typename F>
inline boolean::boolean(std::string label, shared<bool> value, F func)
    : label_(std::move(label)), value_(value, func)
{}

template <typename F>
inline boolean::boolean(std::string label, bool default_value, F func)
    : label_(std::move(label)), value_(default_value, func)
{}

}  // namespace graphene::property
