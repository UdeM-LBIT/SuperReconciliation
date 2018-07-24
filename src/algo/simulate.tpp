#include "simulate.hpp"
#include "../util/numeric.hpp"
#include <algorithm>
#include <boost/container_hash/hash.hpp>
#include <random>
#include <tree.hh>

bool operator==(const SimulationParams& a, const SimulationParams& b)
{
    return
        a.base == b.base
        && a.depth == b.depth
        && a.p_dup == b.p_dup
        && a.p_dup_length == b.p_dup_length
        && a.p_loss == b.p_loss
        && a.p_loss_length == b.p_loss_length
        && a.p_rearr == b.p_rearr;
}

std::hash<SimulationParams>::result_type
std::hash<SimulationParams>::operator()(const argument_type& a) const
{
    result_type seed = boost::hash_range(
        std::begin(a.base),
        std::end(a.base));

    boost::hash_combine(seed, a.depth);
    boost::hash_combine(seed, a.p_dup);
    boost::hash_combine(seed, a.p_dup_length);
    boost::hash_combine(seed, a.p_loss);
    boost::hash_combine(seed, a.p_loss_length);
    boost::hash_combine(seed, a.p_rearr);

    return seed;
}

namespace
{
    /**
     * Randomly get a non-empty segment of a synteny.
     *
     * @param prng Pseudo-random number generator to use.
     * @param size Total size of the synteny.
     * @param p_length Parameter defining the geometric distriubtion of
     * a segment’s length.
     * @return Interval of the randomly-drew segment.
     */
    template<typename PRNG>
    Synteny::Segment get_random_segment(
        PRNG& prng,
        std::size_t size,
        double p_length)
    {
        if (size == 0)
        {
            // No way to get a non-empty segment
            return Synteny::Segment(0, 0);
        }

        // Randomly choose a length for the segment
        std::geometric_distribution<std::size_t> get_length{p_length};
        auto length = clamp(
            get_length(prng) + 1,
            static_cast<std::size_t>(1),
            size);

        // Randomly choose a starting point for the segment from the possible
        // positions given the already chosen length
        std::uniform_int_distribution<std::size_t> get_start{0, size - length};
        auto start = get_start(prng);

        return Synteny::Segment(start, start + length);
    }

    /**
     * Randomly rearrange some pairs of gene families inside a synteny.
     *
     * @param prng Pseudo-random generator to use.
     * @param base Original synteny.
     * @param p_rearr Parameter defining the geometric distribution of the
     * number of rearranged pairs.
     * @return Randomly rearranged synteny.
     */
    template<typename PRNG>
    Synteny get_random_rearrangement(
        PRNG& prng,
        Synteny base,
        double p_rearr)
    {
        std::geometric_distribution<int> choose_pair_count{p_rearr};
        std::uniform_int_distribution<std::size_t> get_index{0, base.size() - 1};

        if (base.size() <= 1)
        {
            // We need at least one pair to be able to rearrange
            return base;
        }

        auto pair_count = choose_pair_count(prng);

        for (int i = 0; i < pair_count; ++i)
        {
            using std::swap;
            auto first = std::next(begin(base), get_index(prng));
            auto second = std::next(begin(base), get_index(prng));

            if (first != second)
            {
                swap(*first, *second);
            }
            else
            {
                // Do not try to swap an element with itself, and draw
                // another pair of indices
                --i;
            }
        }

        return base;
    }

    /**
     * Simulate a sequence of losses and create a tree of loss events
     * recording the loss sequence. This function recurses into
     * `simulate_evolution` whenever it is chosen not to incur losses
     * anymore.
     *
     * @param prng Pseudo-random number generator to use.
     * @param params Parameters for the simulation.
     * @return Simulated event tree.
     */
    template<typename PRNG>
    ::tree<Event> simulate_losses(PRNG& prng, SimulationParams params)
    {
        std::discrete_distribution<int> choose_loss{
            1 - params.p_loss, params.p_loss};

        if (choose_loss(prng) && !params.base.empty())
        {
            // Randomly select a segment to be removed from the synteny
            auto segment = get_random_segment(
                prng, params.base.size(),
                params.p_loss_length);

            Event root;
            root.type = Event::Type::Loss;
            root.synteny = params.base;
            root.segment = segment;

            ::tree<Event> result{std::move(root)};

            // Actually apply the removal before recursing to generate
            // children based on the appropriate synteny
            params.base.erase(
                std::next(std::cbegin(params.base), segment.first),
                std::next(std::cbegin(params.base), segment.second));

            if (!params.base.empty())
            {
                auto child = simulate_losses(prng, std::move(params));
                result.append_child(result.begin(), child.begin());
            }

            return result;
        }
        else
        {
            return simulate_evolution(prng, std::move(params));
        }
    }
}

template<typename PRNG>
::tree<Event> simulate_evolution(PRNG& prng, SimulationParams params)
{
    std::discrete_distribution<int> choose_segment_left_child{0.5, 0.5};
    std::discrete_distribution<int> choose_event_type{
        0,                // probability for None
        params.p_dup,     // probability for Duplication
        1 - params.p_dup, // probability for Speciation
        0};               // probability for Loss

    Event root;
    root.synteny = params.base;

    if (params.base.empty())
    {
        // The synteny has been completely lost: create a full loss node
        root.type = Event::Type::Loss;
        return ::tree<Event>{root};
    }
    else if (params.depth <= 0)
    {
        // We have reached maximum depth: end the branch here
        return ::tree<Event>{root};
    }
    else
    {
        auto type = static_cast<Event::Type>(choose_event_type(prng));
        root.type = type;

        Synteny synteny_left = params.base;
        Synteny synteny_right = params.base;

        // For segmental duplications, either one of the children has a
        // synteny that is a segment of the parent one (this segment
        // may be the whole synteny)
        if (type == Event::Type::Duplication)
        {
            if (choose_segment_left_child(prng))
            {
                auto segment = get_random_segment(
                    prng,
                    synteny_left.size(),
                    params.p_dup_length);

                synteny_left = Synteny(
                    std::next(std::cbegin(synteny_left), segment.first),
                    std::next(std::cbegin(synteny_left), segment.second));

                root.segment = segment;
            }
            else
            {
                auto segment = get_random_segment(
                    prng,
                    synteny_right.size(),
                    params.p_dup_length);

                synteny_right = Synteny(
                    std::next(std::cbegin(synteny_right), segment.first),
                    std::next(std::cbegin(synteny_right), segment.second));

                root.segment = segment;
            }
        }

        // Randomly introduce rearrangements into the child syntenies
        synteny_left = get_random_rearrangement(
            prng, synteny_left,
            params.p_rearr);

        synteny_right = get_random_rearrangement(
            prng, synteny_right,
            params.p_rearr);

        // Randomly introduce losses, which can be cascaded
        SimulationParams params_left = params;
        --params_left.depth;
        params_left.base = synteny_left;

        auto child_left = simulate_losses(prng, params_left);

        SimulationParams params_right = params;
        --params_right.depth;
        params_right.base = synteny_right;

        auto child_right = simulate_losses(prng, params_right);

        // Assemble children and root into the final tree
        ::tree<Event> result{std::move(root)};
        result.append_child(result.begin(), child_left.begin());
        result.append_child(result.begin(), child_right.begin());
        return result;
    }
}
