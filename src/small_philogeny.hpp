#ifndef SMALL_PHILOGENY_HPP
#define SMALL_PHILOGENY_HPP

#include "Event.hpp"
#include "Synteny.hpp"
#include <tree.hh>

/**
 * Compute synteny assignations of internal nodes in a synteny tree so as to
 * minimize the total cost in duplications and segmental losses. This is the
 * “Small Philogeny Problem” applied to syntenies.
 * @param tree Synteny tree in which leaves are labeled with studied syntenies,
 * internal nodes are labeled with events (speciation or duplication) and the
 * root node is labeled with the ancestral synteny to consider. This tree is
 * modified so that the optimal synteny assignation is set in each internal
 * node.
 * @throws If the order is not consistent or if the tree is improperly labeled.
 * @return Cost of the computed optimal synteny assignation (number of
 * segmental duplications and losses).
 */
int small_philogeny(tree<Event>& tree);

#endif // SMALL_PHILOGENY_HPP
