#pragma once

#include <functional>
#include <memory>
#include <shared_mutex>

namespace graphene {

template <typename T>
class shared
{
public:
    shared();

    shared(T value);

    virtual ~shared();

    operator T() const;

    T operator*() const;

    T get() const;

    void operator=(T new_value);

protected:
    struct state;
    std::shared_ptr<state> state_;
};

}  // namespace graphene

#include "shared.ipp"
