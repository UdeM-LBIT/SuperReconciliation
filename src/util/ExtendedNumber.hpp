#ifndef UTIL_EXTENDED_NUMBER_HPP
#define UTIL_EXTENDED_NUMBER_HPP

#include <ostream>

/**
 * Extended number type that can correctly represent positive and negative
 * infinity. This type handles appropriately all operations involving
 * infinities. `std::domain_error` exceptions are thrown whenever the user
 * tries to perform an undefined operation.
 */
template<typename T>
class ExtendedNumber
{
public:
    /**
     * Create an extended number with value 0.
     */
    ExtendedNumber();

    /**
     * Create an extended number with the given value.
     *
     * @param value Value to give to the number.
     */
    ExtendedNumber(const T&);

    /**
     * Create an instance representing positive infinity.
     *
     * @return Instance representing positive infinity.
     */
    static ExtendedNumber positiveInfinity() noexcept;

    /**
     * Create an instance representing negative infinity.
     *
     * @return Instance representing negative infinity.
     */
    static ExtendedNumber negativeInfinity() noexcept;

    /**
     * Check whether the current instance is positive infinity.
     *
     * @return True iff this is positive infinity.
     */
    bool isPositiveInfinity() const noexcept;

    /**
     * Check whether the current instance is negative infinity.
     *
     * @return True iff this is negative infinity.
     */
    bool isNegativeInfinity() const noexcept;

    /**
     * Check whether the current instance is infinity.
     *
     * @return True iff this is infinity.
     */
    bool isInfinity() const noexcept;

    /**
     * Explicitly convert an extended number towards its wrapped type.
     *
     * @throws std::domain_error If this is infinity.
     * @return The wrapped value.
     */
    explicit operator T() const;

    // Comparison operators between extended numbers
    bool operator<(const ExtendedNumber&) const noexcept;
    bool operator==(const ExtendedNumber&) const noexcept;
    bool operator!=(const ExtendedNumber&) const noexcept;
    bool operator<=(const ExtendedNumber&) const noexcept;
    bool operator>(const ExtendedNumber&) const noexcept;
    bool operator>=(const ExtendedNumber&) const noexcept;

    // Unary plus and minus
    ExtendedNumber operator+() const noexcept;
    ExtendedNumber operator-() const noexcept;

    /**
     * Add another extended number to this and stores the result into this.
     *
     * @param rhs Second operand.
     *
     * @throws std::domain_error When trying to add opposite infinities.
     * @return Current instance.
     */
    ExtendedNumber& operator+=(const ExtendedNumber&);
    ExtendedNumber operator+(const ExtendedNumber&) const;

    /**
     * Subtract another extended number from this and stores the result into this.
     *
     * @param rhs Second operand.
     *
     * @throws std::domain_error When trying to subtract same-sign infinities.
     * @return Current instance.
     */
    ExtendedNumber& operator-=(const ExtendedNumber&);
    ExtendedNumber operator-(const ExtendedNumber&) const;

    /**
     * Multiply another extended number to this and stores the result into this.
     *
     * @param rhs Second operand.
     *
     * @throws std::domain_error When trying to multiply zero with infinity.
     * @return Current instance.
     */
    ExtendedNumber& operator*=(const ExtendedNumber&);
    ExtendedNumber operator*(const ExtendedNumber&) const;

    /**
     * Divide another extended number to this and stores the result into this.
     *
     * @param rhs Second operand.
     *
     * @throws std::domain_error When trying to add opposite infinities.
     * @return Current instance.
     */
    ExtendedNumber& operator/=(const ExtendedNumber&);
    ExtendedNumber operator/(const ExtendedNumber&) const;

private:
    // Wrapped value. If infinity, only the sign bit is significative and it
    // indicates whether this is positive or negative infinity.
    T value;

    // True iff this is infinity. In this case, the sign of `value` encodes
    // the sign of infinity.
    bool infinity_flag = false;
};

template<typename T>
bool operator<(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator==(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator!=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator<=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator>(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator>=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
ExtendedNumber<T> operator+(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator-(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator*(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator/(const T&, const ExtendedNumber<T>&);

/**
 * Print an extended number on an output stream.
 *
 * @param out Output stream to print on.
 * @param number Extended number to print.
 *
 * @return Used output stream.
 */
template<typename T>
std::ostream& operator<<(std::ostream&, const ExtendedNumber<T>&);

#include "ExtendedNumber.tpp"

#endif // UTIL_EXTENDED_NUMBER_HPP
