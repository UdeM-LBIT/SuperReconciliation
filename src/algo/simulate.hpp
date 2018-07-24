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
     * Ancestral synteny from which the simulation will evolve.
     */
    Synteny base;

    /**
     * Maximum depth of events on a branch, not counting losses.
     */
    int depth = 5;

    /**
     * Probability for any given internal node to be a duplication.
     */
    double p_dup = 0.5;

    /**
     * Parameter of the geometric distribution of the lengths of segments
     * in segmental duplications.
     */
    double p_dup_length = 0.3;

    /**
     * Probability for a loss under any given speciation node.
     */
    double p_loss = 0.2;

    /**
     * Parameter of the geometric distribution of the lengths of segments
     * in segmental losses.
     */
    double p_loss_length = 0.7;

    /**
     * Parameter of the geometric distribution of the number of gene
     * pairs rearranged from a node to one of its children.
     */
    double p_rearr = 1;
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
