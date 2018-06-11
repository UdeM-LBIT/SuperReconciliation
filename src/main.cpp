#include "Event.hpp"
#include "ExtendedNumber.hpp"
#include "tree_parser.hpp"
#include "util.hpp"
#include <iostream>
#include <limits>
#include <map>
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
int small_philogeny_for_syntenies(tree<Event>& tree, const Synteny& base)
{
    // Exact solution for the problem by a dynamic programming approach,
    // implementing the method described in “Reconstructing the History of
    // Syntenies Through Super-Reconciliation” (El-Mabrouk et al., 2015)

    // Costs (number of segmental duplications and losses) are modeled by an
    // extended integer which enables correct infinity representation
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

        Candidate()
        : cost(0)
        {}

        Candidate(Cost cost)
        : cost(cost)
        {}

        Candidate(Cost cost, Synteny synteny_left, Synteny synteny_right)
        : cost(cost), synteny_left(synteny_left), synteny_right(synteny_right)
        {}
    };

    // List of all possible candidates derived from the ancestral synteny
    auto possibilities = base.generateSubsequences();

    // Data structure storing all candidates for a given node. Here, we
    // associate each candidate synteny (key of the map) to the informations
    // relative to it (value of the map)
    using CandidateMapping = std::map<Synteny, Candidate>;

    // Associate each tree node (event) to its candidate syntenies
    std::map<Event*, CandidateMapping> candidates_per_node;

    // Fill the `candidates_per_node` map with a dynamic programming,
    // bottom-up (postfix order) approach
    for (
        auto it = tree.begin_post();
        it != tree.end_post();
        ++it)
    {
        CandidateMapping candidates;

        if (tree.number_of_children(it) == 0)
        {
            // For leaves, the only possible candidate is the one that is
            // already affected: its cost is 0. We affect to all other
            // candidates an infinite cost so that existing affectations are
            // preserved
            for (const Synteny& candidate : possibilities)
            {
                candidates.emplace(
                    candidate,
                    Candidate{candidate == it->getSynteny()
                        ? 0
                        : Cost::positiveInfinity()});
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

                switch (it->getType())
                {
                case Event::Type::Speciation:
                    // At speciation nodes, only one scenario is possible:
                    // both children were fully copied. If any losses occur,
                    // they are necessarily due to segmental losses following
                    // the speciation event and they have to be counted in the
                    // total cost
                    candidates.emplace(candidate, Candidate{best_total,
                        best_total_left_synteny, best_total_right_synteny});
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
                        candidates.emplace(candidate, Candidate{1 + best_total,
                            best_total_left_synteny, best_total_right_synteny});
                    }
                    else if (best_total_partial <= best_total
                        && best_total_partial <= best_partial_total)
                    {
                        candidates.emplace(candidate, Candidate{1 + best_total_partial,
                            best_total_left_synteny, best_partial_right_synteny});
                    }
                    else if (best_partial_total <= best_total
                        && best_partial_total <= best_total_partial)
                    {
                        candidates.emplace(candidate, Candidate{1 + best_partial_total,
                            best_partial_left_synteny, best_total_right_synteny});
                    }
                    break;

                default:
                {
                    std::ostringstream message;
                    message << "Invalid event type on an internal node: "
                        << it->getType();
                    throw std::invalid_argument{message.str()};
                }
                }
            }
        }

        candidates_per_node.emplace(&*it, candidates);
    }

    // Now that the whole map of candidates has been filled in, in particular
    // for the root node, the optimal candidate for the root node fully
    // determines the optimal assignation for the rest of the tree
    Event* root = &*std::begin(tree);
    auto best_candidate_cost = Cost::positiveInfinity();
    Synteny best_synteny;

    for (const auto& candidate_pair : candidates_per_node.at(root))
    {
        if (candidate_pair.second.cost < best_candidate_cost)
        {
            best_candidate_cost = candidate_pair.second.cost;
            best_synteny = candidate_pair.first;
        }
    }

    if (best_candidate_cost.isInfinity())
    {
        throw std::invalid_argument{"The order of the ancestral synteny "
            "is not consistent with the affectation of at least one of "
            "the leaves."};
    }

    root->setSynteny(best_synteny);

    // It only remains to propagate the best assignations starting from
    // the root node
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        if (tree.number_of_children(it) == 2)
        {
            Event* child_left = &*tree.child(it, 0);
            Event* child_right = &*tree.child(it, 1);

            child_left->setSynteny(candidates_per_node
                .at(&*it).at(it->getSynteny()).synteny_left);
            child_right->setSynteny(candidates_per_node
                .at(&*it).at(it->getSynteny()).synteny_right);
        }
    }

    return static_cast<int>(best_candidate_cost);
}

int main()
{
    std::string ancestral_data;
    std::getline(std::cin, ancestral_data);

    std::istringstream ancestral_tok{ancestral_data};
    std::istream_iterator<Gene> it{ancestral_tok};
    std::istream_iterator<Gene> end;

    Synteny ancestral(it, end);

    std::ostringstream tree_newick;
    tree_newick << std::cin.rdbuf();

    tree<Event> tree = newick_to_tree(tree_newick.str());
    small_philogeny_for_syntenies(tree, ancestral);

    print_tree_dot<Event>(tree, std::cout, [](std::ostream& out, const Event& event)
    {
        out << "shape=\"";

        switch (event.getType())
        {
        case Event::Type::None:
            out << "none";
            break;

        case Event::Type::Duplication:
            out << "box";
            break;

        case Event::Type::Speciation:
            out << "oval";
            break;

        case Event::Type::Loss:
            out << "diamond";
            break;
        }

        out << "\", label=\"" << event.getSynteny() << "\"";
    });
}
