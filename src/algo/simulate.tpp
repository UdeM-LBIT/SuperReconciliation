#include "simulate.hpp"
#include <algorithm>
#include <random>
#include <tree.hh>

namespace detail
{
    template<typename PRNG>
    Synteny simulate_loss(PRNG& prng, Synteny base, double loss_leng_rate)
    {
        if (base.empty())
        {
            return base;
        }

        int total_length = static_cast<int>(base.size());

        std::geometric_distribution<int> get_length{loss_leng_rate};
        auto length = std::clamp(get_length(prng) + 1, 1, total_length);

        std::uniform_int_distribution<int> get_start{0, total_length - length};
        auto it_start = std::next(std::begin(base), get_start(prng));
        auto it_end = std::next(it_start, length);

        base.erase(it_start, it_end);
        return base;
    }

    template<typename PRNG>
    ::tree<Event> simulate_evolution(
        PRNG& prng, Synteny base, int depth,
        double dup_prob, double loss_prob, double loss_leng_rate);

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

        Event root;
        root.synteny = base;

        if (depth > 0)
        {
            root.type = static_cast<Event::Type>(get_event_type(prng));
            ::tree<Event> result{root};

            auto child_left = simulate_losses(
                prng, base, depth - 1,
                dup_prob, loss_prob, loss_leng_rate);

            auto child_right = simulate_losses(
                prng, base, depth - 1,
                dup_prob, loss_prob, loss_leng_rate);

            result.append_child(result.begin(), child_left.begin());
            result.append_child(result.begin(), child_right.begin());

            return result;
        }
        else
        {
            return ::tree<Event>{root};
        }
    }
}

template<typename PRNG>
::tree<Event> simulate_evolution(EvolutionParams<PRNG> params)
{
    return detail::simulate_evolution(
        *params.random_generator, params.base_synteny,
        params.event_depth, params.duplication_probability,
        params.loss_probability, params.loss_length_rate);
}
