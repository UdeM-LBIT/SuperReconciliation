#include "simulate.hpp"
#include "../util/numeric.hpp"
#include <algorithm>
#include <boost/container_hash/hash.hpp>
#include <random>
#include <tree.hh>

bool operator==(const SimulationParams& a, const SimulationParams& b)
{
    return
        a.base_synteny == b.base_synteny
        && a.event_depth == b.event_depth
        && a.duplication_probability == b.duplication_probability
        && a.loss_probability == b.loss_probability
        && a.loss_length_rate == b.loss_length_rate;
}

std::hash<SimulationParams>::result_type
std::hash<SimulationParams>::operator()(const argument_type& a) const
{
    result_type seed = boost::hash_range(
        std::begin(a.base_synteny),
        std::end(a.base_synteny));

    boost::hash_combine(seed, a.event_depth);
    boost::hash_combine(seed, a.duplication_probability);
    boost::hash_combine(seed, a.loss_probability);
    boost::hash_combine(seed, a.loss_length_rate);

    return seed;
}

namespace detail
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
     * @param rearr_rate Parameter defining the geometric distribution of the
     * number of rearranged pairs.
     * @return Randomly rearranged synteny.
     */
    template<typename PRNG>
    Synteny randomly_rearrange(
        PRNG& prng,
        Synteny base,
        double rearr_rate)
    {
        std::geometric_distribution<int> get_no_of_pairs{rearr_rate};
        std::uniform_int_distribution<std::size_t> get_index{0, base.size() - 1};

        if (base.size() <= 1)
        {
            // We need at least one pair to be able to rearrange
            return base;
        }

        auto no_of_pairs = get_no_of_pairs(prng);

        for (int i = 0; i < no_of_pairs; ++i)
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
     * Simulate the evolution of a synteny and generate a tree recording the
     * history of the simulated events. See parameter descriptions in the
     * header file.
     */
    template<typename PRNG>
    ::tree<Event> simulate_evolution(
        PRNG& prng,
        Synteny base,
        int depth,
        double dup_prob,
        double loss_prob,
        double loss_leng_rate,
        double p_dup_length,
        double rearr_rate);

    /**
     * Simulate a sequence of losses and create a tree of loss events
     * recording the loss sequence. This function recurses into
     * `simulate_evolution` whenever it is chosen not to incur losses
     * anymore.
     *
     * @param prng Pseudo-random number generator to use.
     * @param base Root synteny to evolve from.
     * @param depth Maximum depth of events on branch, not counting losses.
     * @param dup_prob Probability for any given internal node to be
     * a duplication.
     * @param loss_prob Probability for a loss under any given speciation node.
     * @param loss_leng_rate Parameter defining the geometric distribution
     * of loss segments’ lengths.
     * @return Simulated event tree.
     */
    template<typename PRNG>
    ::tree<Event> simulate_losses(
        PRNG& prng,
        Synteny base,
        int depth,
        double dup_prob,
        double loss_prob,
        double loss_leng_rate,
        double p_dup_length,
        double rearr_rate)
    {
        std::discrete_distribution<int> choose_if_loss{
            1 - loss_prob, loss_prob};

        if (choose_if_loss(prng) && !base.empty())
        {
            auto segment = get_random_segment(
                prng, base.size(), loss_leng_rate);

            Event root;
            root.type = Event::Type::Loss;
            root.synteny = base;
            root.segment = segment;

            ::tree<Event> result{root};

            base.erase(
                std::next(std::cbegin(base), segment.first),
                std::next(std::cbegin(base), segment.second));

            if (!base.empty())
            {
                auto child = simulate_losses(
                    prng,
                    base,
                    depth,
                    dup_prob,
                    loss_prob,
                    loss_leng_rate,
                    p_dup_length,
                    rearr_rate);

                result.append_child(result.begin(), child.begin());
            }

            return result;
        }
        else
        {
            return simulate_evolution(
                prng,
                base,
                depth,
                dup_prob,
                loss_prob,
                loss_leng_rate,
                p_dup_length,
                rearr_rate);
        }
    }

    template<typename PRNG>
    ::tree<Event> simulate_evolution(
        PRNG& prng,
        Synteny base,
        int depth,
        double dup_prob,
        double loss_prob,
        double loss_leng_rate,
        double p_dup_length,
        double rearr_rate)
    {
        std::discrete_distribution<int> choose_event_type{
            0, dup_prob, 1 - dup_prob, 0};
        std::discrete_distribution<int> choose_segment_left_child{0.5, 0.5};

        Event root;
        root.synteny = base;

        if (base.empty())
        {
            // The synteny has been completely lost: add a full loss node
            root.type = Event::Type::Loss;
            return ::tree<Event>{root};
        }
        else if (depth <= 0)
        {
            // We have reached maximum depth: end the branch here
            return ::tree<Event>{root};
        }
        else
        {
            auto type = static_cast<Event::Type>(choose_event_type(prng));
            root.type = type;

            Synteny synteny_left = base;
            Synteny synteny_right = base;

            // For segmental duplications, either one of the children has a
            // synteny that is a segment of the parent one (this segment
            // may be the whole synteny)
            if (type == Event::Type::Duplication)
            {
                if (choose_segment_left_child(prng))
                {
                    auto segment = get_random_segment(
                        prng, synteny_left.size(), p_dup_length);

                    synteny_left = Synteny(
                        std::next(std::cbegin(synteny_left), segment.first),
                        std::next(std::cbegin(synteny_left), segment.second));

                    root.segment = segment;
                }
                else
                {
                    auto segment = get_random_segment(
                        prng, synteny_right.size(), p_dup_length);

                    synteny_right = Synteny(
                        std::next(std::cbegin(synteny_right), segment.first),
                        std::next(std::cbegin(synteny_right), segment.second));

                    root.segment = segment;
                }
            }

            // Additionally, randomly introduce rearrangements
            synteny_left = randomly_rearrange(prng, synteny_left, rearr_rate);
            synteny_right = randomly_rearrange(prng, synteny_right, rearr_rate);

            // Apply random losses, which can be cascaded
            auto child_left = simulate_losses(
                prng, synteny_left, depth - 1,
                dup_prob, loss_prob, loss_leng_rate,
                p_dup_length, rearr_rate);

            auto child_right = simulate_losses(
                prng, synteny_right, depth - 1,
                dup_prob, loss_prob, loss_leng_rate,
                p_dup_length, rearr_rate);

            ::tree<Event> result{root};
            result.append_child(result.begin(), child_left.begin());
            result.append_child(result.begin(), child_right.begin());
            return result;
        }
    }
}

// Same as detail::simulate_evolution, albeit with a better public interface
template<typename PRNG>
::tree<Event> simulate_evolution(PRNG& prng, SimulationParams params)
{
    return detail::simulate_evolution(
        prng,
        params.base_synteny,
        params.event_depth,
        params.duplication_probability,
        params.loss_probability,
        params.loss_length_rate,
        params.p_dup_length,
        params.rearrangement_rate);
}
