template<class T>
constexpr const T& clamp(const T& value, const T& low, const T& high)
{
    return value < low ? low
        : high < value ? high
        : value;
}
