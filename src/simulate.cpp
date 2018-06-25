#include "algo/simulate.hpp"
#include "util/tree.hpp"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

/**
 * All arguments that can be passed to the simulation program.
 * See below for a description of each argument.
 */
struct Arguments
{
    unsigned synteny_size;
    unsigned event_depth;
    double duplication_probability;
    double loss_probability;
    double loss_length_rate;
};

/**
 * Read arguments passed to the program and produce the
 * help message if requested by the user.
 *
 * @param result Filled with arguments passed to the program or
 * appropriate default values.
 * @param argc Number of arguments in argv.
 * @param argv Tokenized list of arguments passed to the program.
 * @return True if the program may continue, or false if it has
 * to be stopped.
 */
bool read_arguments(Arguments& result, int argc, const char* argv[])
{
    po::options_description root;

    po::options_description gen_opt_group{"General options"};
    gen_opt_group.add_options()
        ("help,h", "show this help message")
    ;
    root.add(gen_opt_group);

    po::options_description sim_opt_group{"Simulation parameters"};
    sim_opt_group.add_options()
        ("synteny-size,s",
         po::value(&result.synteny_size)
            ->value_name("SIZE")
            ->default_value(5),
         "size of the ancestral synteny to evolve from")

        ("depth,d",
         po::value(&result.event_depth)
            ->value_name("SIZE")
            ->default_value(5),
         "maximum depth of events on a branch, not counting losses")

        ("p-dup,D",
         po::value(&result.duplication_probability)
            ->value_name("PROB")
            ->default_value(0.5),
         "probability for any given internal node to be a duplication")

        ("p-loss,L",
         po::value(&result.loss_probability)
            ->value_name("PROB")
            ->default_value(0.5),
         "probability for a loss under any given speciation node")

        ("p-length,R",
         po::value(&result.loss_length_rate)
            ->value_name("PROB")
            ->default_value(0.5),
         "parameter defining the geometric distribution of loss "
         "segments’ lengths")
    ;
    root.add(sim_opt_group);

    po::variables_map values;
    po::store(
        po::command_line_parser(argc, argv)
            .options(root)
            .run(),
        values);

    if (values.count("help"))
    {
        std::cout << "Usage: " << argv[0] << " output -m METRIC [options...]\n";
        std::cout << "\nSimulate the evolution of a ficticious synteny.\n"
            << root;
        return false;
    }

    po::notify(values);
    return true;
}

int main(int argc, const char* argv[])
{
    Arguments args;

    if (!read_arguments(args, argc, argv))
    {
        return EXIT_SUCCESS;
    }

    std::random_device rd;
    std::mt19937 prng{rd()};

    SimulationParams params;
    params.base_synteny = Synteny::generateDummy(args.synteny_size);
    params.event_depth = args.event_depth;
    params.duplication_probability = args.duplication_probability;
    params.loss_probability = args.loss_probability;
    params.loss_length_rate = args.loss_length_rate;

    auto event_tree = simulate_evolution(prng, params);
    auto out_tree = tree_cast<Event, TaggedNode>(event_tree);

    std::cerr << "Simulated evolution tree:\n";
    std::cout << stringify_nhx_tree(out_tree);
    return EXIT_SUCCESS;
}
