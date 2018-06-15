#include "Synteny.hpp"
#include <sstream>
#include <stdexcept>

Synteny Synteny::generateDummy(unsigned long length)
{
    unsigned long i = 0;

    Synteny result;
    Gene current = "a";

    while (i != length)
    {
        result.push_back(current);
        auto incr_pos = current.rbegin();

        while (*incr_pos == 'z' && incr_pos != current.rend())
        {
            *incr_pos = 'a';
            ++incr_pos;
        }

        if (incr_pos == current.rend())
        {
            current.insert(current.begin(), 'a');
        }
        else
        {
            ++*incr_pos;
        }

        ++i;
    }

    return result;
}

std::vector<Synteny> Synteny::generateSubsequences() const
{
    if (this->empty())
    {
        return {{}};
    }

    auto rest = *this;
    rest.pop_front();
    auto result_rest = rest.generateSubsequences();

    std::vector<Synteny> result;

    for (auto subsequence : result_rest)
    {
        result.push_back(subsequence);
        subsequence.push_front(this->front());
        result.push_back(std::move(subsequence));
    }

    return result;
}

int Synteny::distanceTo(const Synteny& target, bool substring) const
{
    auto result = this->reconcile(
        target, ExtendedNumber<int>::positiveInfinity(), substring);
    return result.first;
}

std::pair<int, Synteny> Synteny::reconcile(
    const Synteny& target, ExtendedNumber<int> max, bool substring) const
{
    // Reconciled synteny, that contains at most `max` segment erasures
    // and is closer to `target` than the current synteny in terms of
    // loss distance
    Synteny base = *this;

    auto it_base = std::cbegin(base);
    auto it_target = std::cbegin(target);

    // Minimum number of required segmental losses to turn the
    // [begin(base), it_base) synteny into [begin(target), it_target)
    int amount = 0;

    // Is true iff the genes that precede it_base and it_target have the same
    // value in their respective syntenies. At initialization time, we consider
    // that it is true, to find initial segmental losses
    bool coincides = true;

    // Points on the start of the last lost sequence in the base synteny, if
    // such sequence exists. Otherwise, points to the end of the synteny
    auto it_start = std::cend(base);

    // Iterate on both syntenies to identify lost gene segments
    while (
        amount < max
        && it_base != std::cend(base)
        && it_target != std::cend(target))
    {
        if (*it_base != *it_target)
        {
            if (coincides)
            {
                // Start a new lost segment
                coincides = false;
                it_start = it_base;
            }

            // Advance in the original synteny until the two syntenies
            // coincide again
            ++it_base;
        }
        else
        {
            if (coincides)
            {
                // Advance into a preserved segment
                ++it_base;
                ++it_target;
            }
            else
            {
                // End of a lost segment: count it only if substring
                // mode is disabled
                if (!substring || it_start != std::cbegin(base))
                {
                    ++amount;
                }

                it_base = base.erase(it_start, it_base);
                coincides = true;
            }
        }
    }

    // If the aligned original synteny ends before the new one, the new synteny
    // is not a subsequence of the original
    if (it_base == std::cend(base) && it_target != std::cend(target))
    {
        std::ostringstream message;
        message << "The new synteny (" << target << ") must be a subsequence of"
            " the current one (" << base << ").";
        throw std::invalid_argument{message.str()};
    }

    // If the aligned original synteny ends after the new one, a final segment
    // was lost. We count this segment only if `substring` is false
    if (it_base != std::cend(base) && it_target == std::cend(target))
    {
        if (!substring)
        {
            ++amount;
        }

        base.erase(it_base, std::cend(base));
    }

    return std::make_pair(amount, base);
}

std::string Synteny::difference(const Synteny& target) const
{
    auto it_this = std::cbegin(*this);
    auto it_target = std::cbegin(target);

    std::string result;
    bool coincides = true;

    while (it_this != std::cend(*this) && it_target != std::cend(target))
    {
        if (*it_this != *it_target)
        {
            if (coincides)
            {
                coincides = false;
                result += "[ ";
            }
        }
        else
        {
            if (!coincides)
            {
                coincides = true;
                result += "] ";
            }

            ++it_target;
        }

        result += *it_this;
        ++it_this;

        if (it_this != std::cend(*this))
        {
            result += " ";
        }
    }

    // If the aligned original synteny ends before the new one, the new synteny
    // is not a subsequence of the original
    if (it_this == std::cend(*this) && it_target != std::cend(target))
    {
        std::ostringstream message;
        message << "The new synteny (" << target << ") must be a subsequence of"
            " the current one (" << *this << ").";
        throw std::invalid_argument{message.str()};
    }

    // If the aligned original synteny ends after the new one, a final segment
    // was lost
    if (it_this != std::cend(*this) && it_target == std::cend(target))
    {
        if (coincides)
        {
            result += "[ ";
        }

        while (it_this != std::cend(*this))
        {
            result += *it_this;
            ++it_this;

            if (it_this != std::cend(*this))
            {
                result += " ";
            }
        }

        result += " ]";
    }

    return result;
}

std::ostream& operator<<(std::ostream& out, const Synteny& synteny)
{
    return out << synteny.difference(synteny);
}
