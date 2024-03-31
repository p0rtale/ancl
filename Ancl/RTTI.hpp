#pragma once

#include <cassert>


// TODO: specialize upcasting and cast to same type
template <typename To, typename From>
bool InstanceOf(const From* from) {
    return To::InstanceOf(from);
}

template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

template <typename To, Derived<To> From>
bool InstanceOf(const From* from) {
    return true;
}

template <typename First, typename Second, typename... Rest, typename From>
bool InstanceOf(const From* from) {
    return InstanceOf<First>(from) || InstanceOf<Second, Rest...>(from);
}


template <typename To, typename From>
To* Cast(From* from) {
    assert(InstanceOf<To>(from) && "Incorrect type in Cast");
    return static_cast<To*>(from);
}


template <typename To, typename From>
To* DynamicCast(From* from) {
    if (!InstanceOf<To>(from)) {
        return nullptr;
    }
    return Cast<To>(from);
}
