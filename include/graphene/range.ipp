namespace graphene::property {

template <typename F>
inline
range::range(std::string label, shared<float> value, bounds<float> range, F func)
    : label_(std::move(label)),
      range_(std::move(range)),
      value_(value, func)
{}

template <typename F>
inline
range::range(std::string label, float default_value, bounds<float> range, F func)
    : label_(std::move(label)),
      range_(std::move(range)),
      value_(default_value, func)
{}

} // graphene::property
