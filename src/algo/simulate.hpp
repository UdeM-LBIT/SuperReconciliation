#ifndef ALGO_SIMULATE_HPP
#define ALGO_SIMULATE_HPP

#include "../model/Event.hpp"
#include "../model/Synteny.hpp"

/**
 * Parameters for simulating the evolution of a synteny.
 *
 * @see simulate_evolution
 */
template<typename PRNG>
struct EvolutionParams
{
    /**
     * Pseudo-random number generator to use, from C++’s <random>
     * library generators (eg. std::mt19937).
     */
    PRNG* random_generator = nullptr;

    /**
     * The synteny to evolve from.
     */
    Synteny base_synteny;

    /**
     * Maximum depth of events on a given branch (does not count losses).
     */
    int event_depth = 5;

    /**
     * Probability that a given internal node should be a duplication node.
     */
    double duplication_probability = 0.5;

    /**
     * Probability that a segmental loss should occur under a given
     * internal node.
     */
    double loss_probability = 0.2;

    /**
     * Rate determining the length of lost segments.
     */
    double loss_length_rate = 0.5;
};

/**
 * Simulate the evolution of a synteny.
 *
 * @see EvolutionParams
 * @param params Parameters for the simulation.
 *
 * @return Simulated event tree.
 */
template<typename PRNG>
::tree<Event> simulate_evolution(EvolutionParams<PRNG>);

#include "simulate.tpp"

#endif // ALGO_SIMULATE_HPP
