namespace graphene {

template <typename T>
struct shared<T>::state
{
    state() {}
    state(T value) : value(value) {}

    T value;
    std::shared_mutex mutex;
};

template <typename T>
inline
shared<T>::shared() : state_(new state())
{}

template <typename T>
inline
shared<T>::shared(T value) : state_(new state(value))
{}

template <typename T>
inline shared<T>::~shared()
{}

template <typename T>
inline
shared<T>::operator T() const
{
    std::shared_lock<std::shared_mutex> lock(state_->mutex);
    return state_->value;
}

template <typename T>
inline T
shared<T>::operator*() const
{
    std::shared_lock<std::shared_mutex> lock(state_->mutex);
    return state_->value;
}

template <typename T>
inline T
shared<T>::get() const
{
    std::shared_lock<std::shared_mutex> lock(state_->mutex);
    return state_->value;
}

template <typename T>
inline void
shared<T>::operator=(T new_value)
{
    std::unique_lock<std::shared_mutex> lock(state_->mutex);
    state_->value = new_value;
}

}  // namespace graphene
