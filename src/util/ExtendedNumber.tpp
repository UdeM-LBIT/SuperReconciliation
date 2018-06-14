#include <cmath>
#include <stdexcept>
#include <sstream>

template<typename T>
ExtendedNumber<T>::ExtendedNumber()
: value(0)
{}

template<typename T>
ExtendedNumber<T>::ExtendedNumber(const T& value)
: value(value)
{}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::positiveInfinity() noexcept
{
    ExtendedNumber result = +1;
    result.infinity_flag = true;
    return result;
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::negativeInfinity() noexcept
{
    ExtendedNumber result = -1;
    result.infinity_flag = true;
    return result;
}

template<typename T>
bool ExtendedNumber<T>::isPositiveInfinity() const noexcept
{
    return this->infinity_flag && this->value > 0;
}

template<typename T>
bool ExtendedNumber<T>::isNegativeInfinity() const noexcept
{
    return this->infinity_flag && this->value < 0;
}

template<typename T>
bool ExtendedNumber<T>::isInfinity() const noexcept
{
    return this->infinity_flag;
}

template<typename T>
ExtendedNumber<T>::operator T() const
{
    if (this->infinity_flag)
    {
        std::ostringstream message;
        message << *this << " has no value.";
        throw std::domain_error{message.str()};
    }

    return this->value;
}

template<typename T>
bool ExtendedNumber<T>::operator<(const ExtendedNumber& rhs) const noexcept
{
    if (this->isPositiveInfinity() || rhs.isNegativeInfinity())
    {
        return false;
    }

    if (this->isNegativeInfinity() || rhs.isPositiveInfinity())
    {
        return true;
    }

    return this->value < rhs.value;
}

template<typename T>
bool ExtendedNumber<T>::operator==(const ExtendedNumber& rhs) const noexcept
{
    if (this->infinity_flag && rhs.infinity_flag)
    {
        return this->isPositiveInfinity() == rhs.isPositiveInfinity();
    }

    if (!this->infinity_flag && !rhs.infinity_flag)
    {
        return this->value == rhs.value;
    }

    return false;
}

template<typename T>
bool ExtendedNumber<T>::operator<=(const ExtendedNumber& rhs) const noexcept
{
    return *this == rhs || *this < rhs;
}

template<typename T>
bool ExtendedNumber<T>::operator>(const ExtendedNumber& rhs) const noexcept
{
    return !(*this <= rhs);
}

template<typename T>
bool ExtendedNumber<T>::operator>=(const ExtendedNumber& rhs) const noexcept
{
    return !(*this < rhs);
}

template<typename T>
bool ExtendedNumber<T>::operator!=(const ExtendedNumber& rhs) const noexcept
{
    return !(*this == rhs);
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator+() const noexcept
{
    return *this;
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator-() const noexcept
{
    auto result = *this;
    result.value = -result.value;
    return result;
}

template<typename T>
ExtendedNumber<T>& ExtendedNumber<T>::operator+=(const ExtendedNumber& rhs)
{
    if (!this->infinity_flag && !rhs.infinity_flag)
    {
        this->value += rhs.value;
        return *this;
    }

    if ((this->isPositiveInfinity() && !rhs.isNegativeInfinity())
        || (this->isNegativeInfinity() && !rhs.isPositiveInfinity()))
    {
        return *this;
    }

    if ((rhs.isPositiveInfinity() && !this->isNegativeInfinity())
        || (rhs.isNegativeInfinity() && !this->isPositiveInfinity()))
    {
        *this = rhs;
        return *this;
    }

    std::ostringstream message;
    message << "Operation " << *this << " + " << rhs << " is undefined.";
    throw std::domain_error{message.str()};
}

template<typename T>
ExtendedNumber<T>& ExtendedNumber<T>::operator-=(const ExtendedNumber& rhs)
{
    if (!this->infinity_flag && !rhs.infinity_flag)
    {
        this->value -= rhs.value;
        return *this;
    }

    if ((this->isPositiveInfinity() && !rhs.isPositiveInfinity())
        || (this->isNegativeInfinity() && !rhs.isNegativeInfinity()))
    {
        return *this;
    }

    if ((rhs.isPositiveInfinity() && !this->isPositiveInfinity())
        || (rhs.isNegativeInfinity() && !this->isNegativeInfinity()))
    {
        *this = -rhs;
        return *this;
    }

    std::ostringstream message;
    message << "Operation " << *this << " − " << rhs << " is undefined.";
    throw std::domain_error{message.str()};
}

template<typename T>
ExtendedNumber<T>& ExtendedNumber<T>::operator*=(const ExtendedNumber& rhs)
{
    if (!this->infinity_flag && !rhs.infinity_flag)
    {
        this->value *= rhs.value;
        return *this;
    }

    if (!this->infinity_flag && this->value != 0)
    {
        this->value = rhs.value
            * static_cast<T>(std::copysign(1, this->value));
        this->infinity_flag = true;
        return *this;
    }

    if (!rhs.infinity_flag && rhs.value != 0)
    {
        this->value = this->value
            * static_cast<T>(std::copysign(1, rhs.value));
        this->infinity_flag = true;
        return *this;
    }

    if (this->infinity_flag && rhs.infinity_flag)
    {
        this->value *= rhs.value;
        return *this;
    }

    std::ostringstream message;
    message << "Operation " << *this << " × " << rhs << " is undefined.";
    throw std::domain_error{message.str()};
}

template<typename T>
ExtendedNumber<T>& ExtendedNumber<T>::operator/=(const ExtendedNumber& rhs)
{
    if (!this->infinity_flag && !rhs.infinity_flag && rhs.value != 0)
    {
        this->value /= rhs.value;
        return *this;
    }

    if (!this->infinity_flag && rhs.infinity_flag)
    {
        this->value = 0;
        return *this;
    }

    if (!rhs.infinity_flag && rhs.value != 0)
    {
        this->value = this->value
            * static_cast<T>(std::copysign(1, rhs.value));
        this->infinity_flag = true;
        return *this;
    }

    std::ostringstream message;
    message << "Operation " << *this << " ÷ " << rhs << " is undefined.";
    throw std::domain_error{message.str()};
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator+(const ExtendedNumber& rhs) const
{
    auto result = *this;
    result += rhs;
    return result;
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator-(const ExtendedNumber& rhs) const
{
    auto result = *this;
    result -= rhs;
    return result;
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator*(const ExtendedNumber& rhs) const
{
    auto result = *this;
    result *= rhs;
    return result;
}

template<typename T>
ExtendedNumber<T> ExtendedNumber<T>::operator/(const ExtendedNumber& rhs) const
{
    auto result = *this;
    result /= rhs;
    return result;
}

template<typename T>
bool operator<(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} < rhs;
}

template<typename T>
bool operator==(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} == rhs;
}

template<typename T>
bool operator!=(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} != rhs;
}

template<typename T>
bool operator<=(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} <= rhs;
}

template<typename T>
bool operator>(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} > rhs;
}

template<typename T>
bool operator>=(const T& lhs, const ExtendedNumber<T>& rhs) noexcept
{
    return ExtendedNumber<T>{lhs} >= rhs;
}

template<typename T>
ExtendedNumber<T> operator+(const T& lhs, const ExtendedNumber<T>& rhs)
{
    return ExtendedNumber<T>{lhs} + rhs;
}

template<typename T>
ExtendedNumber<T> operator-(const T& lhs, const ExtendedNumber<T>& rhs)
{
    return ExtendedNumber<T>{lhs} - rhs;
}

template<typename T>
ExtendedNumber<T> operator*(const T& lhs, const ExtendedNumber<T>& rhs)
{
    return ExtendedNumber<T>{lhs} * rhs;
}

template<typename T>
ExtendedNumber<T> operator/(const T& lhs, const ExtendedNumber<T>& rhs)
{
    return ExtendedNumber<T>{lhs} / rhs;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const ExtendedNumber<T>& number)
{
    if (number.isPositiveInfinity())
    {
        return out << "+∞";
    }
    else if (number.isNegativeInfinity())
    {
        return out << "-∞";
    }
    else
    {
        return out << static_cast<T>(number);
    }
}
