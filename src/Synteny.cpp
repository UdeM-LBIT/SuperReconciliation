#include "Synteny.hpp"
#include <stdexcept>

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

int Synteny::distanceTo(const Synteny& to, bool substring) const
{
    auto it_from = std::cbegin(*this);
    auto it_to = std::cbegin(to);

    // Minimum number of required segmental losses to turn the
    // [begin_from, it_from) synteny into [begin_to, it_to)
    int result = 0;

    // Is true iff the genes that precede it_from and it_to in their
    // respective syntenies are equal. At initialization time, we consider
    // that it is true only if we want to take initial segmental losses
    // into account (only when `substring` is false)
    bool coincides = !substring;

    // Iterate on both syntenies to identify lost gene segments
    while (it_from != std::cend(*this) && it_to != std::cend(to))
    {
        if (*it_from != *it_to)
        {
            if (coincides)
            {
                // Start a new segmental loss
                ++result;
                coincides = false;
            }

            // Advance in the original synteny until the two syntenies
            // coincide again
            ++it_from;
        }
        else
        {
            // Advance into a preserved segment
            coincides = true;
            ++it_from;
            ++it_to;
        }
    }

    // If the aligned original synteny ends before the new one, the new synteny
    // is not a subsequence of the original
    if (it_from == std::cend(*this) && it_to != std::cend(to))
    {
        std::ostringstream message;
        message << "The new synteny (" << to << ") must be a subsequence of "
            "the current one (" << *this << ").";
        throw std::invalid_argument{message.str()};
    }

    // If the aligned original synteny ends after the new one, a final segment
    // was lost. We count this segment only if `substring` is false
    if (it_from != std::cend(*this) && it_to == std::cend(to) && !substring)
    {
        ++result;
    }

    return result;
}

std::ostream& operator<<(std::ostream& out, const Synteny& synteny)
{
    for (auto it = std::begin(synteny); it != std::end(synteny); ++it)
    {
        out << *it;

        if (std::next(it) != std::end(synteny))
        {
            out << " ";
        }
    }

    return out;
}
