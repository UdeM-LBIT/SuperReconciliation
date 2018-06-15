#include "../model/Event.hpp"
#include "../util/ExtendedNumber.hpp"
#include <map>
#include <tree.hh>

/**
 * Make sure that, in an event tree, the distance in terms of losses between
 * a parent and one of its children is at most 1 for loss nodes and at most
 * 0 (ie. they have the same synteny) for other nodes by inserting loss nodes
 * where needed.
 *
 * @param tree Tree on which to check the condition.
 * @param parent Parent node.
 * @param child Child node.
 * @param substring Whether to check distances in substring mode or not.
 */
void resolve_losses(
    ::tree<Event>& tree,
    ::tree<Event>::iterator_base parent, ::tree<Event>::iterator_base child,
    bool substring)
{
    auto synteny_parent = parent->synteny;
    auto synteny_child = child->synteny;
    auto distance = synteny_parent.distanceTo(synteny_child, substring);

    // Only loss nodes are allowed to have at most a distance of one with their
    // child syntenies. Other nodes must have exactly the same synteny as their
    // children
    if ((child->type == Event::Type::Loss && distance > 1)
        || (child->type != Event::Type::Loss && distance > 0))
    {
        // If this condition fails for a node, introduce an intermediary loss
        // node between the parent and its faulty child. Recursively resolve
        // discrepancies so that, ultimately, the condition is fulfilled
        Event new_node;
        new_node.type = Event::Type::Loss;
        new_node.synteny
            = synteny_parent.reconcile(synteny_child, 1, substring).second;

        auto new_child = tree.wrap(child, new_node);
        resolve_losses(tree, new_child, child, substring);
    }
}

int super_reconciliation(tree<Event>& tree)
{
    // Exact solution to the problem using a dynamic programming approach,
    // implementing the method described in “Reconstructing the History of
    // Syntenies Through Super-Reconciliation” (El-Mabrouk et al., 2015)

    if (tree.empty())
    {
        return 0;
    }

    // Costs (number of segmental duplications and losses) are modeled by an
    // extended integer which correctly represents infinities
    using Cost = ExtendedNumber<int>;

    // For each node, we call a “candidate synteny” a possible synteny
    // affectation for this node. The following structure stores information
    // relative to a candidate synteny
    struct Candidate
    {
    public:
        // Each candidate has a (potentially infinite) cost. It is the value
        // of d(v, X) as defined in the article, such that v is the node for
        // which X is a candidate synteny
        Cost cost;

        // If this candidate is optimal, then its two optimal child
        // assignations are the following syntenies. If the node is a leaf,
        // then these values are not significant
        Synteny synteny_left;
        Synteny synteny_right;

        // If this is a duplication, the following flags mark whether one
        // of the children were partially duplicated
        bool partial_left = false;
        bool partial_right = false;
    };

    // List of all possible candidates derived from the ancestral synteny
    auto ancestral_synteny = std::begin(tree)->synteny;
    auto possibilities = ancestral_synteny.generateSubsequences();

    // Data structure storing all candidates for a given node. Here, we
    // associate each candidate synteny (key of the map) to the informations
    // relative to it (value of the map)
    using CandidateMapping = std::map<Synteny, Candidate>;

    // Associate each tree node (event) to its candidate syntenies
    std::map<Event*, CandidateMapping> candidates_per_node;

    // Associate each tree node (event) to its best candidate synteny
    std::map<Event*, Synteny> best_candidate_for_node;

    // Fill the `candidates_per_node` map with a dynamic programming,
    // bottom-up (postfix order) approach
    for (
        auto it = tree.begin_post();
        it != tree.end_post();
        ++it)
    {
        CandidateMapping candidates;
        auto best_candidate_cost = ExtendedNumber<int>::positiveInfinity();
        Synteny best_candidate;

        if (tree.number_of_children(it) == 0)
        {
            // For leaves, the only possible candidate is the one that is
            // already affected: its cost is 0. We affect to all other
            // candidates an infinite cost so that existing affectations are
            // preserved
            for (const Synteny& candidate : possibilities)
            {
                Candidate info;
                info.cost = candidate == it->synteny ? 0
                    : Cost::positiveInfinity();

                if (info.cost < best_candidate_cost)
                {
                    best_candidate_cost = info.cost;
                    best_candidate = candidate;
                }

                candidates.emplace(candidate, info);
            }
        }
        else if (tree.number_of_children(it) == 2)
        {
            Event& child_left = *tree.child(it, 0);
            Event& child_right = *tree.child(it, 1);

            for (const Synteny& candidate : possibilities)
            {
                // For each candidate, evaluate the possible candidates that
                // can be affected to the children. These children already have
                // their candidates evaluated because the tree is traversed in
                // postfix order
                auto sub_possibilities = candidate.generateSubsequences();

                // Search for the syntenies that have the least total cost and
                // for the syntenies that have the least partial (substring)
                // cost for each child
                auto best_total_left_cost = Cost::positiveInfinity();
                Synteny best_total_left_synteny;

                auto best_partial_left_cost = Cost::positiveInfinity();
                Synteny best_partial_left_synteny;

                auto best_total_right_cost = Cost::positiveInfinity();
                Synteny best_total_right_synteny;

                auto best_partial_right_cost = Cost::positiveInfinity();
                Synteny best_partial_right_synteny;

                for (const Synteny& sub_candidate : sub_possibilities)
                {
                    auto total_dist = candidate.distanceTo(sub_candidate);
                    auto partial_dist = candidate.distanceTo(sub_candidate, true);

                    auto total_left_cost = total_dist + candidates_per_node
                        .at(&child_left).at(sub_candidate).cost;

                    if (total_left_cost < best_total_left_cost)
                    {
                        best_total_left_cost = total_left_cost;
                        best_total_left_synteny = sub_candidate;
                    }

                    auto partial_left_cost = partial_dist + candidates_per_node
                        .at(&child_left).at(sub_candidate).cost;

                    if (partial_left_cost < best_partial_left_cost)
                    {
                        best_partial_left_cost = partial_left_cost;
                        best_partial_left_synteny = sub_candidate;
                    }

                    auto total_right_cost = total_dist + candidates_per_node
                        .at(&child_right).at(sub_candidate).cost;

                    if (total_right_cost < best_total_right_cost)
                    {
                        best_total_right_cost = total_right_cost;
                        best_total_right_synteny = sub_candidate;
                    }

                    auto partial_right_cost = partial_dist + candidates_per_node
                        .at(&child_right).at(sub_candidate).cost;

                    if (partial_right_cost < best_partial_right_cost)
                    {
                        best_partial_right_cost = partial_right_cost;
                        best_partial_right_synteny = sub_candidate;
                    }
                }

                auto best_total = best_total_left_cost + best_total_right_cost;
                auto best_total_partial
                    = best_total_left_cost + best_partial_right_cost;
                auto best_partial_total
                    = best_partial_left_cost + best_total_right_cost;

                Candidate info;

                switch (it->type)
                {
                case Event::Type::Speciation:
                    // At speciation nodes, only one scenario is possible:
                    // both children were fully copied. If any losses occur,
                    // they are necessarily due to segmental losses following
                    // the speciation event and they have to be counted in the
                    // total cost
                    info.cost = best_total;
                    info.synteny_left = best_total_left_synteny;
                    info.synteny_right = best_total_right_synteny;
                    break;

                case Event::Type::Duplication:
                    // At duplication nodes, we can consider at most one
                    // segmental duplication for one of the two children.
                    // We consider the most advantageous scenario between
                    // a full duplication, a segmental duplication on the left
                    // or a segmental duplication on the right
                    if (best_total <= best_total_partial
                        && best_total <= best_partial_total)
                    {
                        info.cost = 1 + best_total;
                        info.synteny_left = best_total_left_synteny;
                        info.synteny_right = best_total_right_synteny;
                    }
                    else if (best_total_partial <= best_total
                        && best_total_partial <= best_partial_total)
                    {
                        info.cost = 1 + best_total_partial;
                        info.synteny_left = best_total_left_synteny;
                        info.synteny_right = best_partial_right_synteny;
                        info.partial_right = true;
                    }
                    else if (best_partial_total <= best_total
                        && best_partial_total <= best_total_partial)
                    {
                        info.cost = 1 + best_partial_total;
                        info.synteny_left = best_partial_left_synteny;
                        info.partial_left = true;
                        info.synteny_right = best_total_right_synteny;
                    }
                    break;

                default:
                {
                    std::ostringstream message;
                    message << "Invalid event type on an internal node: "
                        << it->type;
                    throw std::invalid_argument{message.str()};
                }
                }

                if (info.cost < best_candidate_cost)
                {
                    best_candidate_cost = info.cost;
                    best_candidate = candidate;
                }

                candidates.emplace(candidate, info);
            } // end loop on candidates
        }

        if (best_candidate_cost.isInfinity())
        {
            std::ostringstream message;
            message << "There is no valid candidate for the node "
                << *it << " under the order of the root synteny ("
                << ancestral_synteny << ").";
            throw std::invalid_argument{message.str()};
        }

        candidates_per_node.emplace(&*it, candidates);
        best_candidate_for_node.emplace(&*it, best_candidate);
    } // end postorder traversal

    // Now that the whole map of candidates has been filled in, in particular
    // for the root node, the optimal candidate for the root node fully
    // determines the optimal assignation for the rest of the tree
    Event* root = &*std::begin(tree);
    root->synteny = best_candidate_for_node.at(root);

    // It only remains to propagate the best assignations starting from
    // the root node
    for (auto parent = tree.begin(); parent != tree.end(); ++parent)
    {
        if (tree.number_of_children(parent) == 2)
        {
            auto synteny_root = parent->synteny;
            auto child_left = tree.child(parent, 0);
            auto child_right = tree.child(parent, 1);
            auto info = candidates_per_node.at(&*parent).at(synteny_root);

            child_left->synteny = info.synteny_left;
            resolve_losses(tree, parent, child_left, info.partial_left);

            child_right->synteny = info.synteny_right;
            resolve_losses(tree, parent, child_right, info.partial_right);
        }
    }

    return static_cast<int>(
        candidates_per_node.at(root).at(root->synteny).cost);
}
