#ifndef UTIL_SET_HPP
#define UTIL_SET_HPP

#include <set>

/**
 * Compute the union set of two sets.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return Set of elements that are either in `lhs` or `rhs`.
 */
template<typename T>
std::set<T> operator+(const std::set<T>& lhs, const std::set<T>& rhs);

/**
 * Compute the difference set of two sets.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return Set of `lhs` elements that are not in `rhs`.
 */
template<typename T>
std::set<T> operator-(const std::set<T>& lhs, const std::set<T>& rhs);

/**
 * Compute the intersection set of two sets.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return Set of elements that are both in `lhs` and `rhs`.
 */
template<typename T>
std::set<T> operator&(const std::set<T>& lhs, const std::set<T>& rhs);

/**
 * Compute the symmetrical difference set of two sets.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return Set of elements that are in one of `lhs` or `rhs` but not in both.
 */
template<typename T>
std::set<T> operator^(const std::set<T>& lhs, const std::set<T>& rhs);

/**
 * Check whether a set is a subset of another.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return True if and only if all elements of `lhs` are in `rhs`.
 */
template<typename T>
bool operator<<(const std::set<T>& lhs, const std::set<T>& rhs);

/**
 * Check whether a set contains another.
 *
 * @param lhs First set.
 * @param rhs Second set.
 * @return True if and only if all elements of `rhs` are in `lhs`.
 */
template<typename T>
bool operator>>(const std::set<T>& lhs, const std::set<T>& rhs);

#include "set.tpp"

#endif // UTIL_SET_HPP
