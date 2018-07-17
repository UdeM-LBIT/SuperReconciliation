#ifndef ALGO_SIMULATE_HPP
#define ALGO_SIMULATE_HPP

#include "../model/Event.hpp"
#include "../model/Synteny.hpp"

/**
 * Parameters for simulating the evolution of a synteny.
 *
 * @see simulate_evolution
 */
struct SimulationParams
{
    /**
     * Root synteny to evolve from.
     */
    Synteny base_synteny;

    /**
     * Maximum depth of events on a branch, not counting losses.
     */
    int event_depth = 5;

    /**
     * Probability for any given internal node to be a duplication.
     */
    double duplication_probability = 0.5;

    /**
     * Probability for a loss under any given speciation node.
     */
    double loss_probability = 0.5;

    /**
     * Parameter defining the geometric distribution of loss segments’ lengths.
     */
    double loss_length_rate = 0.5;

    /**
     * Parameter defining the geometric distribution of the number of gene
     * pairs that can be rearranged from a node to one of its children.
     */
    double rearrangement_rate = 1;
};

bool operator==(const SimulationParams&, const SimulationParams&);

namespace std
{
    template<> struct hash<SimulationParams>
    {
        using argument_type = SimulationParams;
        using result_type = std::size_t;

        result_type operator()(argument_type const& a) const;
    };
}

/**
 * Simulate the evolution of a synteny and generate a tree recording the
 * history of the simulated events.
 *
 * @see SimulationParams
 * @param prng Pseudo-random number generator to use, from C++’s <random>
 * library generators (eg. std::mt19937).
 * @param params Parameters for the simulation.
 *
 * @return Simulated event tree.
 */
template<typename PRNG>
::tree<Event> simulate_evolution(PRNG&, SimulationParams);

#include "simulate.tpp"

#endif // ALGO_SIMULATE_HPP
