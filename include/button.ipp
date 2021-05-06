namespace graphene::property {

template <typename Func>
inline
button::button(const std::string& label, Func func) : label_(label), func_(func)
{}

} // graphene::property
