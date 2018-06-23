#ifndef UTIL_MULTIVALUED_NUMBER_HPP
#define UTIL_MULTIVALUED_NUMBER_HPP

#include <vector>
#include <iostream>

/**
 * Number type that can either hold multiple values at a time or behave
 * exactly like a monovalued number. This is useful for parameters that
 * can be supplied as a single value or a range.
 */
template<typename T, typename Container = std::vector<T>>
class MultivaluedNumber
{
public:
    /**
     * Create a monovalued number with value 0.
     */
    MultivaluedNumber();

    /**
     * Create a monovalued number with the given value.
     *
     * @param value Value to give to the number.
     */
    MultivaluedNumber(const T&);

    /**
     * Create a multivalued number holding all values in the range
     * [`min`, `max`] where `min` and `max` are inclusive, and `step`
     * is the step between each element of the range.
     *
     * @param min Inclusive lower bound for the range.
     * @param max Exclusive upper bound for the range.
     * @param [step=1] Increments between each value.
     */
    MultivaluedNumber(const T&, const T&, const T& = 1);

    /**
     * Create a multivalued number holding all values in a container.
     *
     * @param list List of values to hold.
     */
    MultivaluedNumber(Container);

    /**
     * Check whether this instance contains several numbers.
     */
    bool isMultivalued() const noexcept;

    /**
     * Get an iterator to the beginning of the list of values.
     *
     * @return Iterator to the beginning of the list of values.
     */
    typename Container::iterator begin();
    typename Container::const_iterator cbegin() const;

    /**
     * Get an iterator to the end of the list of values.
     *
     * @return Iterator to the end of the list of values.
     */
    typename Container::iterator end();
    typename Container::const_iterator cend() const;

    /**
     * Explicitly convert a multivalued number to a single value.
     *
     * @throws std::logic_error If this instance does not hold
     * exactly one number.
     * @return The single contained value.
     */
    explicit operator T() const;

    /**
     * Explicitly convert a multivalued number to a list of contained values.
     *
     * @return List of contained values.
     */
    explicit operator Container() const;

    /**
     * Write a multivalued number to an output stream.
     *
     * @param out Output stream to which the output should be directed.
     * @param v Multivalued number to display.
     * @return Used output stream.
     */
    template<typename T_, typename Container_>
    friend std::ostream& operator<<(
        std::ostream&, const MultivaluedNumber<T_, Container_>&);

    /**
     * Extract a multivalued number from an input stream. This can either be
     * a single number, a set of numbers in the format {1, 2, 3} or a range
     * in the format [min : max : step].
     *
     * @param in Input stream from which to extract.
     * @param v Where to store the extracted value.
     * @return Used input stream. Sets failbit on error.
     */
    template<typename T_, typename Container_>
    friend std::istream& operator>>(
        std::istream&, MultivaluedNumber<T_, Container_>&);

private:
    Container values;
};

#include "MultivaluedNumber.tpp"

#endif // UTIL_MULTIVALUED_NUMBER_HPP
