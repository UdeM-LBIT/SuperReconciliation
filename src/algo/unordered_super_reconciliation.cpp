#include "../model/Event.hpp"
#include "../util/ExtendedNumber.hpp"
#include <algorithm>
#include <map>
#include <tree.hh>

namespace
{
/**
 * Perform the initialization pass on the event tree.
 *
 * Compute, for each node, the minimal set of gene families that must
 * be present in its synteny.
 *
 * @param tree Input event tree, in which only the leaves are labelled.
 * @return Dictionary associating each node to its minimal set of genes.
 */
std::map<Event*, std::set<Gene>> initialize(tree<Event>& tree)
{
    std::map<Event*, std::set<Gene>> genes;

    for (
        auto parent = tree.begin_post();
        parent != tree.end_post();
        ++parent)
    {
        if (tree.number_of_children(parent) == 0)
        {
            genes.emplace(
                &*parent,
                std::set<Gene>(
                    cbegin(parent->synteny),
                    cend(parent->synteny)));
        }
        else
        {
            auto genes_left = genes.at(&*tree.child(parent, 0));
            auto genes_right = genes.at(&*tree.child(parent, 1));
            std::set<Gene> genes_union;

            std::set_union(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                inserter(genes_union, end(genes_union)));

            genes.emplace(&*parent, genes_union);
        }
    }

    return genes;
}

/**
 * Perform the propagation pass on the event tree.
 *
 * In prefix order, when the following structure is found in the tree:
 *
 *     x
 *    /\
 *   y …
 *  /\
 * z t
 *
 * where x, y, z and t are all different family sets (note that they are
 * required to be such as z, t ⊊ y ⊊ x), the parent node’s synteny is
 * propagated to its child, yielding the following result:
 *
 *     x
 *    /\
 *   x …
 *  /\
 * z t
 *
 * which saves the introduction of one unnecessary loss.
 *
 * @param tree Input event tree, in which only the leaves are labelled.
 * @param genes Dictionary associating each node to its minimal set of genes.
 */
void propagate(tree<Event>& tree, std::map<Event*, std::set<Gene>>& genes)
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
            if (tree.number_of_children(child) == 2)
            {
                auto child_left = tree.child(child, 0);
                auto child_right = tree.child(child, 1);

                if (
                    genes.at(&*child_left) != genes.at(&*child)
                    && genes.at(&*child_right) != genes.at(&*child)
                    && genes.at(&*child) != genes.at(&*parent)
                )
                {
                    genes.at(&*child) = genes.at(&*parent);
                }
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
 * @param tree Input event tree, in which only the leaves are labelled.
 * @param genes Dictionary associating each node to its minimal set of genes.
 */
void resolve(tree<Event>& tree, std::map<Event*, std::set<Gene>>& genes)
{
    for (
        auto parent = tree.begin_post();
        parent != tree.end_post();
        ++parent)
    {
        if (tree.number_of_children(parent) == 2)
        {
            auto child_left = tree.child(parent, 0);
            auto child_right = tree.child(parent, 1);

            auto genes_parent = genes.at(&*parent);
            auto genes_left = genes.at(&*child_left);
            auto genes_right = genes.at(&*child_right);

            std::set<Gene> gene_union;
            Synteny s1, s2, s3, s4;

            std::set_union(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                inserter(gene_union, end(gene_union)));

            // s1 := left inter right
            std::set_intersection(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                back_inserter(s1));

            // s2 := left \ right
            std::set_difference(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                back_inserter(s2));

            // s3 := parent \ (left union right)
            std::set_difference(
                cbegin(genes_parent), cend(genes_parent),
                cbegin(gene_union), cend(gene_union),
                back_inserter(s3));

            // s4 := right \ left
            std::set_difference(
                cbegin(genes_right), cend(genes_right),
                cbegin(genes_left), cend(genes_left),
                back_inserter(s4));

            auto s1_size = s1.size();
            auto s2_size = s2.size();
            auto s3_size = s3.size();
            auto s4_size = s4.size();

            Synteny synteny_parent = s1;
            Synteny synteny_child_left = s1;
            Synteny synteny_child_right = s1;

            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s2), std::cend(s2));
            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s3), std::cend(s3));
            synteny_parent.insert(
                std::end(synteny_parent),
                std::cbegin(s4), std::cend(s4));

            synteny_child_left.insert(
                std::end(synteny_child_left),
                std::cbegin(s2), std::cend(s2));

            synteny_child_right.insert(
                std::end(synteny_child_right),
                std::cbegin(s4), std::cend(s4));

            parent->synteny = synteny_parent;
            parent->segment = std::make_pair(0, s1_size + s2_size);

            bool is_segmental_left = false;

            if (synteny_child_left != synteny_parent)
            {
                if (parent->type == Event::Type::Duplication)
                {
                    // If the left child differs from its parent, we take
                    // advantage that the parent is a duplication node and
                    // duplicate the `s1 . s2` segment only. This removes
                    // the need to introduce a loss for the left child.
                    is_segmental_left = true;
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
                // If the left child has the exact same synteny as its parent,
                // there are no incurred losses on the left. Therefore, we are
                // free to choose any duplicated segment to better fit the
                // right child. We know that s3 and s4 are empty, therefore
                // the right synteny is `s1`. By chosing to duplicate the `s1`
                // segment, we never incur any loss on the right.
                parent->segment.second = s1_size;
            }
            else if (synteny_child_right != synteny_parent)
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
    auto genes = initialize(tree);
    propagate(tree, genes);
    resolve(tree, genes);
}
