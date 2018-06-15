#include "algo/simulate.hpp"
#include "io/nhx_parser.hpp"
#include "util/tree.hpp"
#include <algorithm>
#include <functional>
#include <random>
#include <tree.hh>

/**
 * Erase loss and internal synteny labelling from a synteny tree.
 *
 * @param tree Input synteny tree.
 * @param root Node from which to start removing.
 * @param [is_root=true] Whether we start removing from the tree’s root.
 *
 * @return Resulting tree.
 */
void erase_tree_info(
    ::tree<Event> input,
    ::tree<Event>::sibling_iterator root,
    bool is_root = true)
{
    switch (root->type)
    {
    case Event::Type::None:
        return;

    case Event::Type::Loss:
    {
        root->synteny = Synteny{};

        if (input.number_of_children(root) != 0)
        {
            auto child = input.child(root, 0);
            input.flatten(root);
            input.erase(root);
            erase_tree_info(input, child, false);
        }

        return;
    }

    case Event::Type::Duplication:
    case Event::Type::Speciation:
        if (!is_root)
        {
            // Erase all syntenies in internal nodes except for the root node
            root->synteny = Synteny{};
        }

        erase_tree_info(input, input.child(root, 0), false);
        erase_tree_info(input, input.child(root, 1), false);
        return;
    }
}

/**
 * Show help.
 */
void help()
{
    std::cout << "simulate – Simulate the evolution of a synteny.\n";
    std::cout << "Arguments:\n\n";
    std::cout << "seed (uint, default = 0)\nNumber used as a seed for the "
        "pseudo-random number generator. If 0, uses a random seed provided by "
        "the system, if available.\n\n";
    std::cout << "length (uint, default = 5)\nLength of the root synteny "
        "to evolve from.\n\n";
    std::cout << "event_depth (uint, default = 5)\nMaximum depth of events"
        "on a given branch of the generated tree, not counting segmental "
        "losses.\n\n";
    std::cout << "duplication_probability (double, default = 0.5)\n"
        "Probability that a given internal node should be a duplication node."
        "\n\n";
    std::cout << "loss_probability (double, default = 0.2)\nProbability "
        "that a segmental loss could occur below a given internal node.\n\n";
    std::cout << "loss_length_rate (double, default = 0.5)\nRate "
        "determining the length of lost segments.\n";
}

int main(int argc, const char* argv[])
{
    // Read program arguments into a vector of strings
    std::vector<std::string> args;
    args.reserve(argc - 1);

    std::transform(
        argv + 1, argv + argc,
        back_inserter(args),
        [](const char* arg) {
            return std::string(arg);
        });

    if (std::find(begin(args), end(args), "--help") != std::end(args))
    {
        help();
        return EXIT_SUCCESS;
    }

    // Fill the evolution simulation parameters from arguments
    unsigned long synteny_length = 5;
    unsigned int seed = 0;

    std::mt19937 prng;
    EvolutionParams<decltype(prng)> params;
    params.random_generator = &prng;

    switch (args.size())
    {
    case 6:
        params.loss_length_rate = stod(args[5]);
        // fallthrough

    case 5:
        params.loss_probability = stod(args[4]);
        // fallthrough

    case 4:
        params.duplication_probability = stod(args[3]);
        // fallthrough

    case 3:
        params.event_depth = stoul(args[2]);
        // fallthrough

    case 2:
        synteny_length = stoul(args[1]);
        // fallthrough

    case 1:
        seed = stoul(args[0]);
        break;
    };

    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::cerr << "Seed: " << seed << "\n";
    prng.seed(seed);

    params.base_synteny = Synteny::generateDummy(synteny_length);

    auto full_tree = simulate_evolution(params);
    auto erased_tree = full_tree;
    erase_tree_info(erased_tree, std::begin(erased_tree));

    auto out_full_tree = tree_cast<Event, TaggedNode>(full_tree);
    auto out_erased_tree = tree_cast<Event, TaggedNode>(erased_tree);

    std::cerr << "Full tree:\n";
    std::cout << stringify_nhx_tree(out_full_tree) << "\n";
    std::cerr << "\nTree with erased information:\n";
    std::cout << stringify_nhx_tree(out_erased_tree) << "\n";
}
