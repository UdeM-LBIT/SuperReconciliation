#include <algorithm>

template<
    typename T,
    template<
        typename,
        typename = std::allocator<T>
    > class Container
>
inline bool contains(const Container<T>& container, const T& value)
{
    return std::find(begin(container), end(container), value)
        != std::end(container);
}
