namespace graphene::property {

template <typename F>
inline rgba::rgba(std::string label, shared<vec4f_t> value, F func)
    : label_(std::move(label)), value_(value, func)
{}

template <typename F>
inline rgba::rgba(std::string label, vec4f_t default_value, F func)
    : label_(std::move(label)), value_(default_value, func)
{}

}  // namespace graphene::property
