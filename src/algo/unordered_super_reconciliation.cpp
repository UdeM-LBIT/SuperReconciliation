#include "../model/Event.hpp"
#include "../util/ExtendedNumber.hpp"
#include <algorithm>
#include <map>
#include <tree.hh>

namespace
{
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

void reduce(tree<Event>& tree, std::map<Event*, std::set<Gene>>& genes)
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

            std::set_intersection(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                back_inserter(s1));

            std::set_difference(
                cbegin(genes_left), cend(genes_left),
                cbegin(genes_right), cend(genes_right),
                back_inserter(s2));

            std::set_difference(
                cbegin(genes_parent), cend(genes_parent),
                cbegin(gene_union), cend(gene_union),
                back_inserter(s3));

            std::set_difference(
                cbegin(genes_right), cend(genes_right),
                cbegin(genes_left), cend(genes_left),
                back_inserter(s4));

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
            bool has_removed = false;

            if (synteny_child_left != synteny_parent)
            {
                if (parent->type == Event::Type::Duplication)
                {
                    has_removed = true;
                }
                else
                {
                    Event loss;
                    loss.type = Event::Type::Loss;
                    loss.synteny = synteny_child_left;
                    tree.wrap(child_left, loss);
                }

            }

            if (synteny_child_right != synteny_parent
                && !(parent->type == Event::Type::Duplication && !has_removed))
            {
                Event loss;
                loss.type = Event::Type::Loss;
                loss.synteny = synteny_child_right;
                tree.wrap(child_right, loss);
            }
        }
    }
}
}

void unordered_super_reconciliation(tree<Event>& tree)
{
    auto genes = initialize(tree);
    reduce(tree, genes);
    resolve(tree, genes);
}
