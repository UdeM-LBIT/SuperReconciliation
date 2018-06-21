#ifndef UTIL_NUMERIC_HPP
#define UTIL_NUMERIC_HPP

/**
 * Emulation of C++17’s std::clamp for unsupporting implementations.
 *
 * @param value Unbounded value to be clamped.
 * @param low Floor value for clamping.
 * @param high Ceil value for clamping.
 * @return Clamped result.
 */
template<class T>
constexpr const T& clamp(const T& value, const T& low, const T& high);

#include "numeric.tpp"

#endif // UTIL_NUMERIC_HPP
