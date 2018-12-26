#pragma once

#include <boost/signals2.hpp>

namespace graphene::detail {

template <typename Signal>
struct signal_wrapper {
    boost::signals2::signal<typename Signal::signature> signal;
};

template <typename... Signals>
struct signal_manager {
    template <typename Signal, typename Func>
    constexpr void
    connect(Func func) {
        std::get<signal_wrapper<Signal>>(signals).signal.connect(func);
    }

    template <typename Signal, typename... Args>
    constexpr void
    emit(Args&&... args) {
        std::get<signal_wrapper<Signal>>(signals).signal(std::forward<Args>(args)...);
    }

    std::tuple<signal_wrapper<Signals>...> signals;
};

} // graphene
