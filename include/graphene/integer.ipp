namespace graphene::property {

template <typename F>
inline
integer::integer(std::string label, shared<int> value, F func)
    : label_(std::move(label)),
      value_(value, func),
      step_(0)
{}

template <typename F>
inline
integer::integer(std::string label, shared<int> value, int step, F func)
    : label_(std::move(label)),
      value_(value, func),
      step_(step)
{}

template <typename F>
inline
integer::integer(std::string label, int default_value, F func)
    : label_(std::move(label)),
      value_(default_value, func),
      step_(0)
{}

template <typename F>
inline
integer::integer(std::string label, int default_value, int step, F func)
    : label_(std::move(label)),
      value_(default_value, func),
      step_(step)
{}

} // graphene::property
