#include "../model/Event.hpp"
#include "../util/ExtendedNumber.hpp"
#include <map>
#include <tree.hh>

namespace
{
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

    // If the parent is a loss node, consider its synteny to be as if the loss
    // had already occurred
    if (parent->type == Event::Type::Loss)
    {
        synteny_parent.erase(
            std::next(std::cbegin(synteny_parent), parent->segment.first),
            std::next(std::cbegin(synteny_parent), parent->segment.second));
    }

    // Edge case: if we happen to generate a internal node which has an empty
    // synteny, we must make sure that it does not have any child, because
    // there can be no evolution from an empty set of genes
    if (synteny_parent.empty())
    {
        tree.erase_children(parent);
        parent->type = Event::Type::Loss;
        return;
    }

    // If the distance between the parent and the child syntenies is at
    // least one, loss nodes need to be introduced between them
    auto losses = synteny_parent.reconcile(synteny_child, substring);

    if (losses.size() >= 1)
    {
        Event new_node;
        new_node.type = Event::Type::Loss;
        new_node.synteny = synteny_parent;
        new_node.segment = losses.front();

        auto new_child = tree.wrap(child, new_node);
        resolve_losses(tree, new_child, child, substring);
        return;
    }
}

Synteny::Segment find_duplicated_segment(
    const Synteny& synteny_parent,
    const Synteny& synteny_child)
{
    auto losses = synteny_parent.reconcile(
        synteny_child, false,
        ExtendedNumber<int>::positiveInfinity());

    auto max = synteny_parent.size();
    auto result = Synteny::Segment(0, max);

    // We are interested in segments at the very start or end of the
    // parent synteny. Those induce a reduction in the duplicated segment
    for (const auto& loss : losses)
    {
        if (loss.first == 0)
        {
            result.first = loss.second;
        }

        if (loss.second == max)
        {
            result.second = loss.first;
        }
    }

    return result;
}

unsigned get_dl_score_helper(tree<Event>& tree, ::tree<Event>::iterator root)
{
    unsigned score = 0;

    if (root->type == Event::Type::Duplication
            || root->type == Event::Type::Loss)
    {
        score = 1;
    }

    for (auto it = tree.begin(root); it != tree.end(root); ++it)
    {
        score += get_dl_score_helper(tree, it);
    }

    return score;
}
}

int get_dl_score(tree<Event>& tree)
{
    return get_dl_score_helper(tree, std::begin(tree));
}

void super_reconciliation(tree<Event>& tree)
{
    // Exact solution to the problem using a dynamic programming approach,
    // implementing the method described in “Reconstructing the History of
    // Syntenies Through Super-Reconciliation” (El-Mabrouk et al., 2015)
    if (tree.empty())
    {
        return;
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
        bool is_consistent = false;
        auto children_count = tree.number_of_children(it);

        if (children_count == 0)
        {
            // For leaves, the only possible candidate is the one that is
            // already affected: its cost is 0. We affect to all other
            // candidates an infinite cost so that existing affectations are
            // preserved
            for (const Synteny& candidate : possibilities)
            {
                Candidate info;
                info.cost = Cost::positiveInfinity();

                if (candidate == it->synteny)
                {
                    info.cost = 0;
                    is_consistent = true;
                }

                candidates.emplace(candidate, info);
            }
        }
        else if (children_count == 2)
        {
            for (const Synteny& candidate : possibilities)
            {
                // For each candidate, evaluate the possible candidates that
                // can be affected to the children. These children already have
                // their candidates evaluated because the tree is traversed in
                // postfix order
                auto sub_possibilities = candidate.generateSubsequences();

                std::vector<Cost> best_total_costs, best_partial_costs;
                std::vector<Synteny> best_total_synts, best_partial_synts;

                best_total_costs.reserve(children_count);
                best_partial_costs.reserve(children_count);
                best_total_synts.reserve(children_count);
                best_partial_costs.reserve(children_count);

                for (auto child = tree.begin(it);
                     child != tree.end(it);
                     ++child)
                {
                    Cost best_total_cost, best_partial_cost;
                    Synteny best_total_synt, best_partial_synt;

                    best_total_cost = best_partial_cost
                        = Cost::positiveInfinity();

                    // Search for the syntenies that have the least total cost
                    // and for the ones that have the least partial cost
                    for (const Synteny& sub_candidate : sub_possibilities)
                    {
                        // The distance to a child loss node is always zero,
                        // because it encodes a loss **from** this
                        // node’s synteny
                        auto total_dist = child->type != Event::Type::Loss
                            ? candidate.distanceTo(sub_candidate) : 0;
                        auto partial_dist = child->type != Event::Type::Loss
                            ? candidate.distanceTo(sub_candidate, true) : 0;

                        auto total_cost = total_dist + candidates_per_node
                            .at(&*child).at(sub_candidate).cost;

                        if (total_cost < best_total_cost)
                        {
                            best_total_cost = total_cost;
                            best_total_synt = sub_candidate;
                        }

                        auto partial_cost = partial_dist + candidates_per_node
                            .at(&*child).at(sub_candidate).cost;

                        if (partial_cost < best_partial_cost)
                        {
                            best_partial_cost = partial_cost;
                            best_partial_synt = sub_candidate;
                        }
                    }

                    best_total_costs.push_back(best_total_cost);
                    best_partial_costs.push_back(best_partial_cost);
                    best_total_synts.push_back(best_total_synt);
                    best_partial_synts.push_back(best_partial_synt);
                } // end loop on children

                auto best_total_total
                    = best_total_costs.at(0) + best_total_costs.at(1);

                auto best_total_partial
                    = best_total_costs.at(0) + best_partial_costs.at(1);

                auto best_partial_total
                    = best_partial_costs.at(0) + best_total_costs.at(1);

                Candidate info;

                switch (it->type)
                {
                case Event::Type::Speciation:
                    // At speciation nodes, only one scenario is possible:
                    // both children were fully copied. If any losses occur,
                    // they are necessarily due to segmental losses following
                    // the speciation event and they have to be counted in the
                    // total cost
                    info.cost = best_total_total;
                    info.synteny_left = best_total_synts.at(0);
                    info.synteny_right = best_total_synts.at(1);
                    break;

                case Event::Type::Duplication:
                    // At duplication nodes, we can consider at most one
                    // segmental duplication for one of the two children.
                    // We consider the most advantageous scenario between
                    // a full duplication, a segmental duplication on the left
                    // or a segmental duplication on the right
                    if (best_total_total <= best_total_partial
                        && best_total_total <= best_partial_total)
                    {
                        info.cost = 1 + best_total_total;
                        info.synteny_left = best_total_synts.at(0);
                        info.synteny_right = best_total_synts.at(1);
                    }
                    else if (best_total_partial <= best_total_total
                        && best_total_partial <= best_partial_total)
                    {
                        info.cost = 1 + best_total_partial;
                        info.synteny_left = best_total_synts.at(0);
                        info.synteny_right = best_partial_synts.at(1);
                        info.partial_right = true;
                    }
                    else if (best_partial_total <= best_total_total
                        && best_partial_total <= best_total_partial)
                    {
                        info.cost = 1 + best_partial_total;
                        info.synteny_left = best_partial_synts.at(0);
                        info.partial_left = true;
                        info.synteny_right = best_total_synts.at(1);
                    }
                    break;

                default:
                {
                    std::ostringstream message;
                    message << "Invalid event type on an internal node: "
                        << it->type;
                    throw std::invalid_argument{message.str()};
                }
                } // end switch on node type

                if (!info.cost.isInfinity())
                {
                    is_consistent = true;
                }

                candidates.emplace(candidate, info);
            } // end loop on candidates
        }

        if (!is_consistent)
        {
            std::ostringstream message;
            message << "There is no valid candidate for the node "
                << *it << " under the order of the root synteny ("
                << ancestral_synteny << ").";
            throw std::invalid_argument{message.str()};
        }

        candidates_per_node.emplace(&*it, candidates);
    } // end postorder traversal

    // We know, for each node, a list of candidates and their associated cost.
    // Each candidate fully determines the optimal assignation for the subtree
    // below it. For the root node, we already know the optimal assignation: it
    // is the one that was already assigned. Thus, it only remains to propagate
    // the best assignations starting from the root node
    for (auto parent = tree.begin(); parent != tree.end(); ++parent)
    {
        if (tree.number_of_children(parent) == 2)
        {
            auto synteny_parent = parent->synteny;
            auto child_left = tree.child(parent, 0);
            auto child_right = tree.child(parent, 1);
            auto info = candidates_per_node.at(&*parent).at(synteny_parent);

            if (info.partial_left)
            {
                parent->segment = find_duplicated_segment(
                    synteny_parent,
                    info.synteny_left);
            }

            if (info.partial_right)
            {
                parent->segment = find_duplicated_segment(
                    synteny_parent,
                    info.synteny_right);
            }

            child_left->synteny = info.synteny_left;
            resolve_losses(tree, parent, child_left, info.partial_left);

            child_right->synteny = info.synteny_right;
            resolve_losses(tree, parent, child_right, info.partial_right);
        }
    }
}
