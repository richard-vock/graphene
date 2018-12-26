namespace graphene::property {

template <typename F>
inline
real::real(std::string label, shared<float> value, uint32_t precision, F func)
    : label_(std::move(label)),
      value_(value, func),
      precision_(precision),
      step_(0.f)
{}

template <typename F>
inline
real::real(std::string label, shared<float> value, uint32_t precision, float step, F func)
    : label_(std::move(label)),
      value_(value, func),
      precision_(precision),
      step_(step)
{}

template <typename F>
inline
real::real(std::string label, float default_value, uint32_t precision, F func)
    : label_(std::move(label)),
      value_(default_value, func),
      precision_(precision),
      step_(0.f)
{}

template <typename F>
inline
real::real(std::string label, float default_value, uint32_t precision, float step, F func)
    : label_(std::move(label)),
      value_(default_value, func),
      precision_(precision),
      step_(step)
{}

} // graphene::property
