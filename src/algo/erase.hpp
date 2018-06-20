#ifndef ALGO_ERASE_HPP
#define ALGO_ERASE_HPP

#include <tree.hh>
#include "../model/Event.hpp"

/**
 * Erase loss and internal synteny labelling from a synteny tree.
 *
 * @param tree Input synteny tree.
 * @param root Node from which to start removing.
 * @param [is_root=true] Whether `root` is the root of the whole tree or of
 * one of the subtrees.
 *
 * @return Resulting tree.
 */
void erase_tree(
    ::tree<Event> input,
    ::tree<Event>::sibling_iterator root,
    bool is_root = true);

#endif // ALGO_ERASE_HPP
