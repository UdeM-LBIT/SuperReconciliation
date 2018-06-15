#include "model/Event.hpp"
#include "io/nhx_parser.hpp"
#include "util/tree.hpp"
#include <algorithm>
#include <functional>
#include <random>
#include <tree.hh>

template<typename PRNG>
Synteny random_losses(PRNG& prng, Synteny base,
    double p_loss, double p_advance)
{
    std::geometric_distribution<int> loss_length_distr{p_loss};
    auto loss_length = std::bind(loss_length_distr, std::ref(prng));

    std::geometric_distribution<int> advance_length_distr{p_advance};
    auto advance_length = std::bind(advance_length_distr, std::ref(prng));

    auto it = std::begin(base);

    while (it != std::end(base))
    {
        auto loss = std::clamp(loss_length(), 0,
            static_cast<int>(std::distance(it, std::end(base))));
        it = base.erase(it, std::next(it, loss));

        auto advance = std::clamp(advance_length() + 1, 0,
            static_cast<int>(std::distance(it, std::end(base))));
        std::advance(it, advance);
    }

    return base;
}

template<typename PRNG>
tree<Event> generate_random_tree(PRNG& prng, Synteny root, int max_depth,
    double p_leaf, double p_dupl, double p_loss, double p_advance)
{
    std::discrete_distribution<int> force_leaf_distr{1 - p_leaf, p_leaf};
    auto force_leaf = std::bind(force_leaf_distr, std::ref(prng));

    std::discrete_distribution<int> node_type_distr{0, p_dupl, 1 - p_dupl, 0};
    auto node_type = std::bind(node_type_distr, std::ref(prng));

    if (force_leaf() || root.size() <= 1 || max_depth <= 0)
    {
        // Make a leaf node
        if (root.empty())
        {
            Event node;
            node.type = Event::Type::Loss;
            node.synteny = root;
            return tree<Event>{node};
        }
        else
        {
            Event node;
            node.synteny = root;
            return tree<Event>{node};
        }
    }
    else
    {
        // Make an internal node
        auto event_type = static_cast<Event::Type>(node_type());
        Event node;
        node.type = event_type;
        tree<Event> result{node};

        auto left_node = result.append_child(result.begin());
        auto left_synteny = random_losses(prng, root, p_loss, p_advance);
        auto left_subtree = generate_random_tree(
            prng, left_synteny, max_depth - 1,
            p_leaf, p_dupl, p_loss, p_advance);
        result.move_ontop(left_node, left_subtree.begin());

        auto right_node = result.append_child(result.begin());
        auto right_synteny = random_losses(prng, root, p_loss, p_advance);
        auto right_subtree = generate_random_tree(
            prng, right_synteny, max_depth - 1,
            p_leaf, p_dupl, p_loss, p_advance);
        result.move_ontop(right_node, right_subtree.begin());

        return result;
    }
}

void help(std::ostream& out)
{
    out << "Usage: ./random root max_depth p_loss p_advance p_leaf "
        "p_dupl seed\nGenerate a random input synteny tree.\n\n";
    out << "- root : number of genes in the root synteny (uint, default : 6)\n";
    out << "- max_depth : maximum depth of the tree (uint, default : 10)\n";
    out << "- p_loss : during random loss creation, probability of "
        "keeping a gene (double, default : 0.6)\n";
    out << "- p_advance : during random loss creation, probability of "
        "not skipping a gene from being deleted (double, default : 0.4)\n";
    out << "- p_leaf : probability for a given node to be a leaf (double, "
        "default : 0.1)\n";
    out << "- p_dupl : probability for an internal node to be a duplication"
        " node (double, default : 0.5)\n";
    out << "- seed : seed number for the pseudo-random number generator "
        "(uint, default : random seed from the system)\n";
}

int main(int argc, const char* argv[])
{
    unsigned int root_size = 6;
    unsigned int max_depth = 10;
    double p_loss = 0.6;
    double p_advance = 0.4;
    double p_leaf = 0.1;
    double p_dupl = 0.5;

    std::random_device rd;
    unsigned int seed = rd();

    std::vector<std::string> args;
    args.reserve(argc);

    std::transform(
        argv, argv + argc,
        back_inserter(args),
        [](const char* arg) {
            return std::string(arg);
        });

    switch (args.size())
    {
    case 8:
        seed = stoul(args[7], nullptr, 10);
        // fallthrough

    case 7:
        p_dupl = std::stod(args[6]);
        // fallthrough

    case 6:
        p_leaf = std::stod(args[5]);
        // fallthrough

    case 5:
        p_advance = std::stod(args[3]);
        // fallthrough

    case 4:
        p_loss = std::stod(args[4]);
        // fallthrough

    case 3:
        max_depth = std::stoul(args[2]);
        // fallthrough

    case 2:
        root_size = std::stoul(args[1]);
        break;
    };

    std::cerr << "Seed: " << seed << "\n\n";
    std::mt19937 prng{seed};

    Synteny root;

    for (char it = 'a'; it < 'a' + root_size; ++it)
    {
        root.push_back(std::string{it});
    }

    auto event_tree = generate_random_tree(
        prng, root, max_depth, p_leaf, p_dupl, p_loss, p_advance);

    std::begin(event_tree)->synteny = root;

    auto output_tree = tree_cast<Event, TaggedNode>(event_tree);
    std::cout << stringify_nhx_tree(output_tree);
}
