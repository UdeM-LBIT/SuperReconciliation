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
    unsigned seed;
    unsigned base_size;
    unsigned depth;
    double p_dup;
    double p_dup_length;
    double p_loss;
    double p_loss_length;
    double p_rearr;

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
        ("seed,S",
         po::value(&result.seed)
            ->value_name("SEED")
            ->default_value(0),
         "seed for the pseudo-random number generator. The special value 0 "
         "instructs the program to grab a random seed from one of the "
         "system’s entropy sources")

        ("base-size,s",
         po::value(&result.base_size)
            ->value_name("SIZE")
            ->default_value(5),
         "number of genes in the ancestral synteny from which the "
         "simulation will evolve")

        ("depth,H",
         po::value(&result.depth)
            ->value_name("SIZE")
            ->default_value(5),
         "maximum depth of events on a branch, not counting losses")

        ("p-dup,d",
         po::value(&result.p_dup)
            ->value_name("PROB")
            ->default_value(0.5),
         "probability for any given internal node to be a duplication")

        ("p-dup-length,D",
         po::value(&result.p_dup_length)
            ->value_name("PROB")
            ->default_value(0.3, "0.3"),
         "parameter of the geometric distribution of the lengths of "
         "segments in segmental duplications")

        ("p-loss,l",
         po::value(&result.p_loss)
            ->value_name("PROB")
            ->default_value(0.2),
         "probability for a loss under any given speciation node")

        ("p-loss-length,L",
         po::value(&result.p_loss_length)
            ->value_name("PROB")
            ->default_value(0.7, "0.7"),
         "parameter of the geometric distribution of the lengths of "
         "segments in segmental losses")

        ("p-rearr,R",
         po::value(&result.p_rearr)
            ->value_name("PROB")
            ->default_value(1),
         "parameter of the geometric distribution of the number of gene "
         "pairs rearranged from a node to one of its children (for example "
         ", if 1, no pair is ever rearranged)")
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

    unsigned seed = args.seed;

    if (seed == 0)
    {
        seed = std::random_device()();
    }

    std::mt19937 prng{seed};

    SimulationParams params;
    params.base = Synteny::generateDummy(args.base_size);
    params.depth = args.depth;
    params.p_dup = args.p_dup;
    params.p_dup_length = args.p_dup_length;
    params.p_loss = args.p_loss;
    params.p_loss_length = args.p_loss_length;
    params.p_rearr = args.p_rearr;

    auto event_tree = simulate_evolution(prng, params);
    auto result_tree = tree_cast<Event, TaggedNode>(event_tree);

    write_all_to(
        args.output_path,
        stringify_nhx_tree(result_tree),
        "Simulated evolution tree:");

    return EXIT_SUCCESS;
}
