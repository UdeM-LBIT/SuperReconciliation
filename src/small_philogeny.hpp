#ifndef SMALL_PHILOGENY_HPP
#define SMALL_PHILOGENY_HPP

#include "Event.hpp"
#include "Synteny.hpp"
#include <tree.hh>

/**
 * Compute synteny assignations of internal nodes in a synteny tree so as to
 * minimize the total cost in duplications and segmental losses. This is the
 * “Small Philogeny Problem” applied to syntenies.
 * @param tree Synteny tree in which leaves are already labeled with studied
 * syntenies and internal nodes are labeled with events (speciation or
 * duplication). This tree is modified so that the optimal synteny assignation
 * is set in each internal node.
 * @param base Ancestral synteny, which labels the root.
 * @throws If the order is not consistent.
 * @return Cost of the computed optimal synteny assignation (number of
 * segmental duplications and losses).
 */
int small_philogeny(tree<Event>& tree, const Synteny& base);

#endif // SMALL_PHILOGENY_HPP
