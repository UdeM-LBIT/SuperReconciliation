#include "algo/super_reconciliation.hpp"
#include "algo/unordered_super_reconciliation.hpp"
#include "io/nhx.hpp"
#include "io/util.hpp"
#include "util/tree.hpp"
#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace po = boost::program_options;

/**
 * All arguments that can be passed to the program.
 * See below for a description of each argument.
 */
struct Arguments
{
    bool use_unordered;
    std::string input_path;
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
    po::options_description root{"General options"};
    root.add_options()
        ("help,h", "show this help message")
        ("unordered,U",
         po::bool_switch(&result.use_unordered),
         "use the unordered super-reconciliation algorithm")
        ("input,I",
         po::value(&result.input_path)
            ->value_name("PATH")
            ->default_value("-"),
         "path of the file from which to read the input tree, or '-' to "
            "read it from standard input")
        ("output,o",
         po::value(&result.output_path)
            ->value_name("PATH")
            ->default_value("-"),
         "path of the file in which the output tree should be stored, or '-' "
            "to store it in standard output")
    ;

    po::variables_map values;
    po::store(
        po::command_line_parser(argc, argv)
            .options(root)
            .run(),
        values);

    if (values.count("help"))
    {
        std::cout << "Usage: " << argv[0] << " [options...]\n";
        std::cout << "\nCompute a super-reconciliation of an input tree.\n";
        std::cout << root;
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

    auto input_tree = parse_nhx_tree(read_all_from(
        args.input_path,
        "Input the tree to be reconciled "
            "and finish with Ctrl-D:"));

    auto event_tree = tree_cast<TaggedNode, Event>(input_tree);

    if (args.use_unordered)
    {
        unordered_super_reconciliation(event_tree);
    }
    else
    {
        super_reconciliation(event_tree);
    }

    auto result_tree = tree_cast<Event, TaggedNode>(event_tree);

    write_all_to(
        args.output_path,
        stringify_nhx_tree(result_tree),
        "Reconciled tree (use `viz` to visualize):");

    return EXIT_SUCCESS;
}
