#ifndef UTIL_CONTAINERS_HPP
#define UTIL_CONTAINERS_HPP

template<
    typename T,
    template<
        typename,
        typename = std::allocator<T>
    > class Container
>
inline bool contains(const Container<T>& container, const T& value);

#include "containers.tpp"

#endif // UTIL_CONTAINERS_HPP
