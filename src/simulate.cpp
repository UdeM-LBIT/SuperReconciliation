#include "algo/simulate.hpp"
#include "util/tree.hpp"
#include "io/util.hpp"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

/**
 * All arguments that can be passed to the program.
 * See below for a description of each argument.
 */
struct Arguments
{
    unsigned synteny_size;
    unsigned event_depth;
    double duplication_probability;
    double loss_probability;
    double loss_length_rate;
    double rearrangement_rate;

    std::string output_path;
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
        ("output,o",
         po::value(&result.output_path)
            ->value_name("PATH")
            ->default_value("-"),
         "path of the file in which the simulated tree should be stored, "
            "or '-' to store it in standard output")
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

        ("p-rearr,S",
         po::value(&result.rearrangement_rate)
            ->value_name("PROB")
            ->default_value(1),
         "parameter defining the geometric distribution of the number "
         "of gene pairs that can be rearranged from a node to one of its "
         "children (eg., if 1, never rearranges any pair)")
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
        std::cout << "Usage: " << argv[0] << " [options...]\n";
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
    params.rearrangement_rate = args.rearrangement_rate;

    auto event_tree = simulate_evolution(prng, params);
    auto result_tree = tree_cast<Event, TaggedNode>(event_tree);

    write_all_to(
        args.output_path,
        stringify_nhx_tree(result_tree),
        "Simulated evolution tree:");

    return EXIT_SUCCESS;
}
