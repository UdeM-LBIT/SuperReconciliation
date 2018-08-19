#include "../model/Event.hpp"
#include "../util/ExtendedNumber.hpp"
#include "../util/set.hpp"
#include <algorithm>
#include <map>
#include <tree.hh>

namespace
{
/**
 * Hold genes and propagation information regarding a nodes in a tree
 * (see the three passes below for a more in-depth explanation).
 */
struct TreeInfoValue
{
    // Contain the set of genes that must be present in the final
    // synteny of this node so that the labeling is valid and minimal.
    std::set<Gene> genes;

    // Signal that this node should receive the same set of genes as
    // its parent node because it would result in less losses.
    bool should_propagate;
};

using TreeInfo = std::map<Event*, TreeInfoValue>;

/**
 * Perform the initialization pass on the event tree.
 *
 * Compute, for each node, the minimal set of gene families that must
 * be present in its synteny and whether it should propagate or not.
 *
 * @param tree Input event tree, in which only the leaves are labelled.
 * @return Dictionary associating each node to its genes and propagation
 * information. In this pass, the gene sets are only the minimal sets
 * required for the labeling to be valid.
 */
TreeInfo initialize(tree<Event>& tree)
{
    TreeInfo info;

    for (
        auto parent = tree.begin_post();
        parent != tree.end_post();
        ++parent)
    {
        if (tree.number_of_children(parent) == 0)
        {
            info.emplace(&*parent, TreeInfoValue{
                // A leaf simply contains all the genes that it was labeled
                // with in the input
                std::set<Gene>(
                    cbegin(parent->synteny),
                    cend(parent->synteny)),

                // No leaves should be ever modified, so we should not
                // propagate on them
                /* should_propagate = */ false
            });
        }
        else if (tree.number_of_children(parent) == 1)
        {
            throw new std::invalid_argument{"Unexpected unary node."};
        }
        else
        {
            auto child_left = tree.child(parent, 0);
            const auto& info_left = info.at(&*child_left);

            auto child_right = tree.child(parent, 1);
            const auto& info_right = info.at(&*child_right);

            auto genes_union = info_left.genes + info_right.genes;

            info.emplace(&*parent, TreeInfoValue{
                // An internal node must always contain all the genes that
                // must belong to its children
                genes_union,

                // All cases in which it is more advantageous to propagate
                // the parent synteny to this node
                // TODO (mdelabre): refine those conditions
                /* should_propagate = */
                ((info_left.genes != genes_union
                  || info_left.should_propagate)
                 && (info_right.genes != genes_union
                     || info_right.should_propagate))
                || (parent->type == Event::Type::Duplication
                        && (child_left->type == Event::Type::Loss
                            || info_left.should_propagate
                            || child_right->type == Event::Type::Loss
                            || info_right.should_propagate))
                || ((info_left.should_propagate
                            || child_left->type == Event::Type::Loss)
                        && (info_right.should_propagate
                            || child_right->type == Event::Type::Loss))
            });
        }
    }

    return info;
}

/**
 * Perform the propagation pass on the event tree.
 *
 * For any node x which must be propagated, x’s parent synteny is copied
 * as x’s synteny.
 *
 * @param tree Input event tree, in which only the leaves are labelled.
 * @param info Dictionary associating each node to its genes and propagation
 * information. After this pass, the gene sets minimize the number of losses
 * that must be introduced by the resolution pass to make the labeling valid.
 */
void propagate(tree<Event>& tree, TreeInfo& info)
{
    for (
        auto parent = tree.begin();
        parent != tree.end();
        ++parent)
    {
        for (
            auto child = tree.begin(parent);
            child != tree.end(parent);
            ++child)
        {
            if (info.at(&*child).should_propagate)
            {
                info.at(&*child).genes = info.at(&*parent).genes;
            }
        }
    }
}

/**
 * Perform the resolution pass on the event tree.
 *
 * Use the minimal gene set dictionary to infer valid syntenic orders
 * while introducing a minimal number of losses. Modify the tree to
 * insert the required losses and set syntenies.
 *
 * @param tree Input event tree, in which only the leaves are labelled. After
 * this pass, all internal nodes are correctly labeled, losses are introduced
 * where necessary and duplicated segments are specified.
 * @param info Dictionary associating each node to its genes information.
 */
void resolve(tree<Event>& tree, TreeInfo& info)
{
    for (
        auto parent = tree.begin_post();
        parent != tree.end_post();
        ++parent)
    {
        const auto& genes_parent = info.at(&*parent).genes;

        // Edge case: if we happen to find an internal node whose minimal
        // set of families is empty, we can safely discard all its children
        // because there can be no evolution from an empty set of genes
        if (genes_parent.empty())
        {
            tree.erase_children(parent);
            parent->type = Event::Type::Loss;
        }
        else if (tree.number_of_children(parent) == 2)
        {
            auto child_left = tree.child(parent, 0);
            const auto& genes_left = info.at(&*child_left).genes;

            auto child_right = tree.child(parent, 1);
            const auto& genes_right = info.at(&*child_right).genes;

            auto s1 = genes_left & genes_right;
            auto s1_size = s1.size();

            auto s2 = genes_left - genes_right;
            auto s2_size = s2.size();

            auto s3 = genes_parent - (genes_left + genes_right);
            auto s3_size = s3.size();

            auto s4 = genes_right - genes_left;
            auto s4_size = s4.size();

            // parent := s1 . s2 . s3 . s4
            Synteny synteny_parent{std::cbegin(s1), std::cend(s1)};
            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s2), std::cend(s2));
            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s3), std::cend(s3));
            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s4), std::cend(s4));

            // left := s1 . s2
            Synteny synteny_left{std::cbegin(s1), std::cend(s1)};
            synteny_left.insert(
                std::end(synteny_left),
                std::cbegin(s2), std::cend(s2));

            // right := s1 . s4
            Synteny synteny_right{std::cbegin(s1), std::cend(s1)};
            synteny_right.insert(
                std::end(synteny_right),
                std::cbegin(s4), std::cend(s4));

            parent->synteny = synteny_parent;
            bool is_segmental_left = false;

            if (synteny_left != synteny_parent
                    && child_left->type != Event::Type::Loss)
            {
                if (parent->type == Event::Type::Duplication)
                {
                    // If the left child differs from its parent, we take
                    // advantage that the parent is a duplication node and
                    // duplicate the `s1 . s2` segment only. This removes
                    // the need to introduce a loss for the left child.
                    is_segmental_left = true;
                    parent->segment = std::make_pair(0, s1_size + s2_size);
                }
                else
                {
                    Event loss;
                    loss.type = Event::Type::Loss;
                    loss.synteny = synteny_parent;
                    loss.segment = std::make_pair(
                        s1_size + s2_size,
                        s1_size + s2_size + s3_size + s4_size);

                    tree.wrap(child_left, loss);
                }
            }

            if (parent->type == Event::Type::Duplication && !is_segmental_left)
            {
                // If the left child has the exact same synteny as its parent
                // (or is a full loss), there are no additional incurred losses
                // on the left. Therefore, we are free to choose any duplicated
                // segment to better fit the right child.
                if (child_left->type == Event::Type::Loss)
                {
                    // If the left child is a full loss, `s1` is necessarily
                    // empty, therefore `right = s4`
                    parent->segment = std::make_pair(
                        s1_size + s2_size + s3_size,
                        s1_size + s2_size + s3_size + s4_size);
                }
                else
                {
                    // If the left child is exactly equal to its parent, `s4`
                    // is necessarily empty, therefore `right = s1`
                    parent->segment = std::make_pair(0, s1_size);
                }
            }
            else if (synteny_right != synteny_parent
                    && child_right->type != Event::Type::Loss)
            {
                Event loss;
                loss.type = Event::Type::Loss;
                loss.synteny = synteny_parent;
                loss.segment = std::make_pair(
                    s1_size,
                    s1_size + s2_size + s3_size);
                tree.wrap(child_right, loss);
            }
        }
    }
}
}

void unordered_super_reconciliation(tree<Event>& tree)
{
    auto info = initialize(tree);
    propagate(tree, info);
    resolve(tree, info);
}
