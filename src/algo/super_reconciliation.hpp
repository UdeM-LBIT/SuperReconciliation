#ifndef ALGO_SUPER_RECONCILIATION_HPP
#define ALGO_SUPER_RECONCILIATION_HPP

#include "../model/Event.hpp"
#include "../model/Synteny.hpp"
#include <tree.hh>

/**
 * Compute synteny assignations of internal nodes in a synteny tree so as to
 * minimize the total cost in duplications and segmental losses. This is the
 * “Super-Reconciliation” problem as described in “Reconstructing the History of
 * Syntenies Through Super-Reconciliation” (El-Mabrouk et al., 2015).
 *
 * @param tree Synteny tree in which leaves are labeled with studied syntenies,
 * internal nodes are labeled with events (speciation or duplication) and the
 * root node is labeled with the ancestral synteny to consider. This tree is
 * modified so that the optimal synteny assignation is set in each internal
 * node.
 *
 * @throws If the order is not consistent or if the tree is improperly labeled.
 * @return Cost of the computed optimal synteny assignation (number of
 * segmental duplications and losses).
 */
int super_reconciliation(tree<Event>& tree);

#endif // ALGO_SUPER_RECONCILIATION_HPP
