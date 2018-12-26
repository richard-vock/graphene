namespace graphene::property {

template <typename T>
inline
tracked<T>::tracked(shared<T> value) : value_(value) {}

template <typename T>
template <typename F>
inline
tracked<T>::tracked(shared<T> value, F func) : value_(value), func_(func) {}

template <typename T>
template <typename F>
inline
tracked<T>::tracked(T default_value, F func)
    : value_(default_value),
      func_(std::move(func))
{}

template <typename T>
inline
tracked<T>::operator T() const
{
    return value_;
}

template <typename T>
inline tracked<T>&
tracked<T>::operator =(T new_value)
{
    value_ = new_value;
    if (func_) {
        func_(value_);
    }

    return *this;
}

} // graphene::property
