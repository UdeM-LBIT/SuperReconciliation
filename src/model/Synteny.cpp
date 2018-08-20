#include "Synteny.hpp"
#include <sstream>
#include <stdexcept>

constexpr Synteny::Segment Synteny::NoSegment;

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
    return this->reconcile(
        target,
        substring,
        ExtendedNumber<int>::positiveInfinity()).size();
}

std::vector<Synteny::Segment> Synteny::reconcile(
    const Synteny& target,
    bool substring,
    ExtendedNumber<int> max
) const
{
    // Reconciled synteny, that contains at most `max` segment erasures
    // and is closer to `target` than the current synteny in terms of
    // loss distance
    auto it_base = std::cbegin(*this);
    auto it_target = std::cbegin(target);

    // List of lost segments that have to be removed to turn the
    // [begin(*this), it_base) synteny into [begin(target), it_target)
    std::vector<Synteny::Segment> lost_segments;

    // Is true iff the genes that precede it_base and it_target have the same
    // value in their respective syntenies. At initialization time, we consider
    // that it is true, to find initial segmental losses
    bool coincides = true;

    // Points on the start of the last lost sequence in the base synteny, if
    // such a sequence exists. Otherwise, points to the end of the synteny
    auto it_start = std::cend(*this);

    // Iterate on both syntenies to identify lost gene segments
    while (
        static_cast<int>(lost_segments.size()) < max
        && it_base != std::cend(*this)
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
                // End of a lost segment: if at the start,
                // count it only if substring mode is disabled
                if (!substring || it_start != std::cbegin(*this))
                {
                    lost_segments.emplace_back(
                        std::distance(std::cbegin(*this), it_start),
                        std::distance(std::cbegin(*this), it_base));
                }

                coincides = true;
            }
        }
    }

    // If the aligned original synteny ends before the new one, the new synteny
    // is not a subsequence of the original
    if (it_base == std::cend(*this) && it_target != std::cend(target))
    {
        std::ostringstream message;
        message << "The new synteny (" << target << ") must be a subsequence of"
            " the current one (" << *this << ").";
        throw std::invalid_argument{message.str()};
    }

    // If the aligned original synteny ends after the new one, a final segment
    // was lost. We count this segment only if substring mode is disabled
    if (it_base != std::cend(*this) && it_target == std::cend(target))
    {
        if (!substring)
        {
            lost_segments.emplace_back(
                std::distance(std::cbegin(*this), it_base),
                this->size());
        }
    }

    return lost_segments;
}

std::ostream& operator<<(std::ostream& out, const Synteny& synteny)
{
    for (
        auto it = std::begin(synteny);
        it != std::end(synteny);
        ++it)
    {
        out << *it;

        if (std::next(it) != std::end(synteny))
        {
            out << " ";
        }
    }

    return out;
}
