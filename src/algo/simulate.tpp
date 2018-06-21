#include "simulate.hpp"
#include "../util/numeric.hpp"
#include <algorithm>
#include <random>
#include <tree.hh>

namespace detail
{
    /**
     * Get a random segment of a synteny.
     *
     * @param prng Pseudo-random number generator to use.
     * @param base Original synteny.
     * @param loss_leng_rate Parameter defining the geometric distribution
     * of the segment’s start and end points.
     * @return A random segment.
     */
    template<typename PRNG>
    Synteny get_random_segment(PRNG& prng, Synteny base, double offset_rate)
    {
        // Use a half less frequent loss because we eat from both sides
        double half_rate = (1 + offset_rate) / 2;

        std::geometric_distribution<std::size_t> get_start_offset{half_rate};
        std::geometric_distribution<std::size_t> get_end_offset{half_rate};

        auto total_length = base.size();
        auto start_offset = get_start_offset(prng);
        auto end_offset = get_end_offset(prng);

        if (start_offset >= total_length || end_offset >= total_length
            || start_offset >= total_length - end_offset - 1)
        {
            return Synteny{};
        }

        return Synteny{
            std::next(std::begin(base), start_offset),
            std::prev(std::end(base), end_offset)};
    }

    /**
     * Simulate the loss of a continuous gene family segment on a synteny.
     *
     * @param prng Pseudo-random number generator to use.
     * @param base Original synteny.
     * @param loss_leng_rate Parameter defining the geometric distribution
     * of loss segments’ lengths.
     * @return New synteny with a lost segment.
     */
    template<typename PRNG>
    Synteny simulate_loss(PRNG& prng, Synteny base, double loss_leng_rate)
    {
        if (base.empty())
        {
            return base;
        }

        int total_length = static_cast<int>(base.size());

        // Randomly choose a length for the loss, and add 1 if the loss
        // is to be forced
        std::geometric_distribution<int> get_length{loss_leng_rate};
        auto length = clamp(get_length(prng) + 1, 1, total_length);

        // Randomly choose a starting point for the loss from the possible
        // positions given the already chosen length
        std::uniform_int_distribution<int> get_start{0, total_length - length};
        auto it_start = std::next(std::begin(base), get_start(prng));
        auto it_end = std::next(it_start, length);

        base.erase(it_start, it_end);
        return base;
    }

    /**
     * Simulate the evolution of a synteny and generate a tree recording the
     * history of the simulated events.
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
    ::tree<Event> simulate_evolution(
        PRNG& prng, Synteny base, int depth,
        double dup_prob, double loss_prob, double loss_leng_rate);

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
        PRNG& prng, Synteny base, int depth,
        double dup_prob, double loss_prob, double loss_leng_rate)
    {
        std::discrete_distribution<int> get_incur_loss{
            1 - loss_prob, loss_prob};

        if (get_incur_loss(prng) && !base.empty())
        {
            Event root;
            root.type = Event::Type::Loss;
            root.synteny = simulate_loss(prng, base, loss_leng_rate);

            ::tree<Event> result{root};

            if (!root.synteny.empty())
            {
                auto child = simulate_losses(
                    prng, root.synteny, depth,
                    dup_prob, loss_prob, loss_leng_rate);

                result.append_child(result.begin(), child.begin());
            }

            return result;
        }
        else
        {
            return simulate_evolution(
                prng, base, depth,
                dup_prob, loss_prob, loss_leng_rate);
        }
    }

    template<typename PRNG>
    ::tree<Event> simulate_evolution(
        PRNG& prng, Synteny base, int depth,
        double dup_prob, double loss_prob, double loss_leng_rate)
    {
        std::discrete_distribution<int> get_event_type{
            0, dup_prob, 1 - dup_prob, 0};
        std::discrete_distribution<int> is_left_child{0.5, 0.5};

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
            auto type = static_cast<Event::Type>(get_event_type(prng));
            root.type = type;
            ::tree<Event> result{root};

            ::tree<Event> child_left;
            ::tree<Event> child_right;

            switch (type)
            {
            case Event::Type::Duplication:
            {
                // Segmental duplication: either one of the children
                // has a synteny that is a segment of the parent one
                Synteny synteny_left = base;
                Synteny synteny_right = base;

                if (is_left_child(prng))
                {
                    synteny_left = get_random_segment(
                        prng, synteny_left, loss_leng_rate);
                }
                else
                {
                    synteny_right = get_random_segment(
                        prng, synteny_right, loss_leng_rate);
                }

                child_left = simulate_evolution(
                    prng, synteny_left, depth - 1,
                    dup_prob, loss_prob, loss_leng_rate);

                child_right = simulate_evolution(
                    prng, synteny_right, depth - 1,
                    dup_prob, loss_prob, loss_leng_rate);
                break;
            }

            case Event::Type::Speciation:
                // Speciation: losses are decided at random and can be cascaded.
                // However, contrary to segmental duplications, any loss causes
                // the creation of a loss node
                child_left = simulate_losses(
                    prng, base, depth - 1,
                    dup_prob, loss_prob, loss_leng_rate);

                child_right = simulate_losses(
                    prng, base, depth - 1,
                    dup_prob, loss_prob, loss_leng_rate);
                break;

            default:
                break;
            }

            result.append_child(result.begin(), child_left.begin());
            result.append_child(result.begin(), child_right.begin());

            return result;
        }
    }
}

// Same as detail::simulate_evolution, albeit with a better public interface
template<typename PRNG>
::tree<Event> simulate_evolution(EvolutionParams<PRNG> params)
{
    return detail::simulate_evolution(
        *params.random_generator, params.base_synteny,
        params.event_depth, params.duplication_probability,
        params.loss_probability, params.loss_length_rate);
}
