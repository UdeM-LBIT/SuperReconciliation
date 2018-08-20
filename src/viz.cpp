#include "io/nhx.hpp"
#include "io/util.hpp"
#include "util/tree.hpp"
#include "model/Synteny.hpp"
#include "model/Event.hpp"
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
         "path of the file in which the output should be stored, or '-' "
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
        std::cout << "\nCreate a Graphviz-compatible representation of a "
            "tree.\n";
        std::cout << root;
        return false;
    }

    po::notify(values);
    return true;
}

std::string event_to_graphviz(const Event& event)
{
    std::string result;

    switch (event.type)
    {
    case Event::Type::Loss:
        result += "fontcolor=\"red\", ";
        // fall-through

    case Event::Type::None:
        result += "shape=\"none\", ";
        break;

    case Event::Type::Duplication:
        result += "shape=\"box\", ";
        break;

    case Event::Type::Speciation:
        result += "shape=\"oval\", ";
        break;
    }

    result += "label=<";

    std::size_t index = 0;
    auto it = std::begin(event.synteny);

    while (true)
    {
        if (event.segment.first != event.segment.second)
        {
            if (index == event.segment.first)
            {
                if (it != std::begin(event.synteny))
                {
                    result += " ";
                }

                switch (event.type)
                {
                case Event::Type::Duplication:
                    result += "<u>";
                    break;

                case Event::Type::Loss:
                    result += "[";
                    break;

                default:
                    break;
                }
            }

            if (index == event.segment.second)
            {
                switch (event.type)
                {
                case Event::Type::Duplication:
                    result += "</u>";
                    break;

                case Event::Type::Loss:
                    result += "]";
                    break;

                default:
                    break;
                }
            }
        }

        if (it == std::end(event.synteny))
        {
            break;
        }

        if (index != event.segment.first && it != std::begin(event.synteny))
        {
            result += " ";
        }

        result += *it;

        ++it;
        ++index;
    }

    result += ">";
    return result;
}

std::string event_subtree_to_graphviz(
    const tree<Event>& tree,
    typename ::tree<Event>::iterator root)
{
    std::string result;

    for (typename ::tree<Event>::sibling_iterator it = tree.begin(root);
            it != tree.end(root); ++it)
    {
        result += "    "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*root))
            + " -- "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*it));

        if (it->type == Event::Type::Loss && it->synteny.empty())
        {
            result += " [style=dashed]";
        }

        result += ";\n" + event_subtree_to_graphviz(tree, it);
    }

    return result;
}

std::string event_tree_to_graphviz(const tree<Event>& tree)
{
    std::string result = "graph {\n";

    for (
        auto it = tree.begin();
        it != tree.end();
        ++it)
    {
        auto parent_node = tree.parent(it);
        Synteny parent_synteny = it->synteny;

        // If we are considering the root of the tree, it has no parent node
        if (tree.is_valid(parent_node))
        {
            parent_synteny = parent_node->synteny;
        }

        // We use a node’s address in memory as a unique identifier
        result += "    "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*it))
            + " [" + event_to_graphviz(*it) + "];\n";
    }

    for (typename ::tree<Event>::sibling_iterator it = tree.begin();
            it != tree.end(); ++it)
    {
        result += event_subtree_to_graphviz(tree, it);
    }

    result += "}\n";
    return result;
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
        "Input the tree to be converted to a Graphviz representation, "
            "and finish with Ctrl-D:"));

    auto event_tree = tree_cast<TaggedNode, Event>(input_tree);

    write_all_to(
        args.output_path,
        event_tree_to_graphviz(event_tree),
        "Tree in Graphviz format (can be piped into `dot`):");

    return EXIT_SUCCESS;
}
