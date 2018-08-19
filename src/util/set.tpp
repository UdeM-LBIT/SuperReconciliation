#include <set>
#include <algorithm>

template<typename T>
std::set<T> operator+(const std::set<T>& lhs, const std::set<T>& rhs)
{
    std::set<T> result;
    std::set_union(
            std::cbegin(lhs), std::cend(lhs),
            std::cbegin(rhs), std::cend(rhs),
            std::inserter(result, std::end(result)));
    return result;
}

template<typename T>
std::set<T> operator-(const std::set<T>& lhs, const std::set<T>& rhs)
{
    std::set<T> result;
    std::set_difference(
            std::cbegin(lhs), std::cend(lhs),
            std::cbegin(rhs), std::cend(rhs),
            std::inserter(result, std::end(result)));
    return result;
}

template<typename T>
std::set<T> operator&(const std::set<T>& lhs, const std::set<T>& rhs)
{
    std::set<T> result;
    std::set_intersection(
            std::cbegin(lhs), std::cend(lhs),
            std::cbegin(rhs), std::cend(rhs),
            std::inserter(result, std::end(result)));
    return result;
}

template<typename T>
std::set<T> operator^(const std::set<T>& lhs, const std::set<T>& rhs)
{
    std::set<T> result;
    std::set_symmetric_difference(
            std::cbegin(lhs), std::cend(lhs),
            std::cbegin(rhs), std::cend(rhs),
            std::inserter(result, std::end(result)));
    return result;
}

template<typename T>
bool operator<<(const std::set<T>& lhs, const std::set<T>& rhs)
{
    return std::includes(
            std::cbegin(rhs), std::cend(rhs),
            std::cbegin(lhs), std::cend(lhs));
}

template<typename T>
bool operator>>(const std::set<T>& lhs, const std::set<T>& rhs)
{
    return !(lhs << rhs);
}
