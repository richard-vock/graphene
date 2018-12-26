namespace graphene {

template <typename T>
explicit active(T)->active<T>;

template <typename T>
explicit option(const char*, T)->option<T>;

template <typename T>
explicit option(std::string, T)->option<T>;

namespace property {

namespace detail {

template <typename T>
inline option<T>
to_option(std::variant<option<T>, active<option<T>>> opt)
{
    if (std::holds_alternative<active<option<T>>>(opt)) {
        return std::get<active<option<T>>>(opt).value;
    }
    return std::get<option<T>>(opt);
}

void
render_line();

bool
render_radio_button(const std::string& label, bool state);

}  // namespace detail

template <typename T>
template <typename... Ts>
inline choice<T>::choice(std::string label, shared<T> value,
                         option<T> possible, Ts&&... more)
    : label_(label), value_(value), text_(label)
{
    auto push = [&](option<T> v) {
        labels_.push_back(v.label);
        possible_.push_back(v.value);
    };
    std::apply([&](auto&&... el) { (push(el), ...); },
               std::make_tuple(possible, std::forward<Ts>(more)...));

    if (auto find_it = std::find(possible_.begin(), possible_.end(), *value);
        find_it != possible_.end()) {
        active_ = std::distance(possible_.begin(), find_it);
    } else {
        throw std::runtime_error("Choice must contain its initial value.");
    }
}

template <typename T>
template <typename F, typename... Ts>
inline choice<T>::choice(std::string label, F func,
                         std::variant<option<T>, active<option<T>>> possible,
                         Ts&&... more)
    : label_(label),
      value_(detail::to_option(possible).value, func),
      text_(label)
{
    active_ = 0;
    uint32_t idx = 0;
    auto push = [&](std::variant<option<T>, active<option<T>>> v) {
        if (std::holds_alternative<active<option<T>>>(v)) {
            active_ = idx;
        }
        option<T> opt = detail::to_option(v);
        labels_.push_back(opt.label);
        possible_.push_back(opt.value);
        ++idx;
    };
    std::apply([&](auto&&... el) { (push(el), ...); },
               std::make_tuple(possible, std::forward<Ts>(more)...));
}

template <typename T>
inline void
choice<T>::render()
{
    if (label_ != "") {
        text_.render();
    }
    for (uint32_t i = 0; i < labels_.size(); ++i) {
        if (i) {
            detail::render_line();
        }

        if (detail::render_radio_button(labels_[i], i == active_)) {
            value_ = possible_[i];
            active_ = i;
        }
    }
}

}  // namespace property

}  // namespace graphene
