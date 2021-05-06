namespace graphene::property {

template <typename F>
inline
rgb::rgb(std::string label, shared<vec3f_t> value, F func)
    : label_(std::move(label)),
      value_(value, func)
{}

template <typename F>
inline
rgb::rgb(std::string label, vec3f_t default_value, F func)
    : label_(std::move(label)), value_(default_value, func)
{}

} // graphene::property
